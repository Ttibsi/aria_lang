#include <string.h>
#include <unistd.h>

#define NOB_STRIP_PREFIX
#define NOB_IMPLEMENTATION
#include "include/nob.h"

#define ARIA_C_VER "-std=gnu23"
#define ARIA_EXE "aria"

int usage() {
    fprintf(stderr, "Valid commands: clean\n");
    return 0;
}

void print_cmd(Cmd* cmd) {
    String_Builder sb = {0};
    cmd_render(*cmd, &sb);
    sb_append_null(&sb);
    nob_log(NOB_WARNING, "%s\n", sb.items);
    exit(1);
}

void mkdir_all() {
    mkdir_if_not_exists("build/");
    mkdir_if_not_exists("build/objects");
}

const char* file_extension(const char* path) {
    const char* slash = strrchr(path, '/');
    const char* dot = strrchr(path, '.');
    if (!dot || (slash && dot < slash) || dot == path) return NULL;
    return dot + 1;
}

const char* strip_src_prefix(const char* path) {
    const char* prefix = "src/";
    const size_t n = strlen(prefix);
    if (strncmp(path, prefix, n) == 0) { return path + n; }
    return path;
}

void sb_append_rel_stem_from_src(String_Builder* sb, const char* path) {
    const char* rel = strip_src_prefix(path);
    const char* ext = file_extension(rel);
    size_t stem_len = ext ? (size_t)((ext - 1) - rel) : strlen(rel);
    sb_append_buf(sb, rel, stem_len);
}

bool walker(Nob_Walk_Entry entry) {
    // TODO: Handle checking timestamps to prevent recompilation when not needed
    // TODO: explore running async to speed up compilation when needed

    if (entry.type == NOB_FILE_DIRECTORY) {
        String_Builder pathName = {0};
        sb_append_cstr(&pathName, "build/objects/");
        sb_append_cstr(&pathName, strip_src_prefix(entry.path));
        sb_append_null(&pathName);

        mkdir_if_not_exists(pathName.items);

        sb_free(pathName);
        return true;

    } else if (entry.type == NOB_FILE_REGULAR) {
        const char* ext = file_extension(entry.path);
        if (!ext || strcmp(ext, "c") != 0) { return true; }

        String_Builder objName = {0};
        sb_append_cstr(&objName, "build/objects/");
        sb_append_rel_stem_from_src(&objName, entry.path);
        sb_append_cstr(&objName, ".o");
        sb_append_null(&objName);

        Cmd cmd = {0};
        cmd_append(&cmd, "gcc");
        nob_cc_flags(&cmd);
        cmd_append(&cmd, "-g");
        cmd_append(&cmd, ARIA_C_VER);
        cmd_append(&cmd, "-Isrc");
        cmd_append(&cmd, "-c");
        cmd_append(&cmd, entry.path);
        cmd_append(&cmd, "-o");
        cmd_append(&cmd, objName.items);

        return cmd_run(&cmd);
    }

    return true;
}

bool ends_with(const char* s, const char* suffix) {
    size_t n = strlen(s);
    size_t m = strlen(suffix);
    if (m > n) return false;
    return memcmp(s + (n - m), suffix, m) == 0;
}

bool compile_static_object(const char* build_root, const char* objects_root) {
    Nob_File_Paths subdirs = {0};
    if (!read_entire_dir(objects_root, &subdirs)) return false;

    for (size_t i = 0; i < subdirs.count; i++) {
        const char* dir_name = subdirs.items[i];
        if (strcmp(dir_name, ".") == 0 || strcmp(dir_name, "..") == 0) continue;

        const char* subdir_path = temp_sprintf("%s%c%s", objects_root, '/', dir_name);

        if (get_file_type(subdir_path) != FILE_DIRECTORY) { continue; }

        // Gather *.o files in this subdir (one level)
        Nob_File_Paths files = {0};
        if (!read_entire_dir(subdir_path, &files)) {
            free(files.items);
            free(subdirs.items);
            return false;
        }

        File_Paths objects = {0};
        for (size_t j = 0; j < files.count; j++) {
            const char* name = files.items[j];
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
            if (!ends_with(name, ".o")) continue;

            const char* obj_path = temp_sprintf("%s%c%s", subdir_path, '/', name);

            if (get_file_type(obj_path) == FILE_REGULAR) { da_append(&objects, obj_path); }
        }

        free(files.items);

        if (objects.count == 0) {
            nob_log(NOB_INFO, "No .o files in %s; skipping", subdir_path);
            free(objects.items);
            continue;
        }

        // Output archive at top-level build/: build/lib<dir>.a
        const char* archive_path = temp_sprintf("%s%clib%s.a", build_root, '/', dir_name);

        Cmd cmd = {0};
        cmd_append(&cmd, "ar", "rcs", archive_path);
        for (size_t k = 0; k < objects.count; k++) { cmd_append(&cmd, objects.items[k]); }

        nob_log(NOB_INFO, "AR %s (%zu objects)", archive_path, objects.count);
        if (!cmd_run(&cmd)) {
            free(objects.items);
            free(subdirs.items);
            return false;
        }

        free(objects.items);
    }

    free(subdirs.items);
    return true;
}

bool collect_archives(Nob_Walk_Entry entry) {
    if (entry.type == FILE_DIRECTORY) {
        if (entry.level > 0) { *entry.action = WALK_SKIP; }

        return true;
    }

    if (entry.type != FILE_REGULAR) { return true; }
    if (entry.level != 1) { return true; }

    Cmd* cmd = (Cmd*)entry.data;

    const char* ext = file_extension(entry.path);
    if (ext && strcmp(ext, "a") == 0) { cmd_append(cmd, temp_sprintf("%s", entry.path)); }

    return true;
}

int compile_exe() {
    Cmd exec_cmd = {0};
    cmd_append(&exec_cmd, "gcc");

    if (access("build/objects/main.o", F_OK) == 0) {
        cmd_append(&exec_cmd, "build/objects/main.o");
    }

    walk_dir("build", collect_archives, .data = &exec_cmd);
    cmd_append(&exec_cmd, "-o", "build/" ARIA_EXE);

    cmd_run(&exec_cmd);
    return 0;
}

bool clean() {
    Cmd cmd = {0};
    cmd_append(&cmd, "rm", "-rf", "build", "nob.old");
    return cmd_run(&cmd);
}

bool test() {
    if (!walk_dir("src/", walker)) { return 1; }
    if (compile_static_object("build", "build/objects") == 0) { return 1; }

    Cmd cmd = {0};
    cmd_append(&cmd, "gcc");
    cmd_append(&cmd, ARIA_C_VER);
    cmd_append(&cmd, "tests/test.c");

    walk_dir("build", collect_archives, .data = &cmd);

    cmd_append(&cmd, "-Isrc");
    cmd_append(&cmd, "-Iinclude");
    cmd_append(&cmd, "-o", "build/test_exe");

    return cmd_run(&cmd);
}

bool build_main_binary() {
    if (!walk_dir("src/", walker)) { return 1; }

    if (compile_static_object("build", "build/objects") == 0) { return 1; }
    nob_log(NOB_INFO, "Object compilation complete");
    if (compile_exe() != 0) { return 1; }

    return 0;
}

int main(int argc, char** argv) {
    GO_REBUILD_URSELF(argc, argv);
    mkdir_all();

    if (argc > 1) {
        if (strcmp(argv[1], "clean") == 0) { return !clean(); }
        if (strcmp(argv[1], "test") == 0) { return !test(); }

        return usage();
    }

    if (build_main_binary()) { return 1; }
    return 0;
}
