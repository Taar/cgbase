#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h>
#include <sys/stat.h>

#define BUFFER_SIZE 512
static_assert(BUFFER_SIZE >= 64, "Buffer size is too small");

static FILE* log_file = NULL;

int init_logging(const char* filename) {
    if (filename == NULL) {
        perror("Invalid file name");
        return -1;
    }
    log_file = fopen(filename, "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return -1;
    }
    int chmod_error = chmod(filename, S_IRUSR | S_IWUSR);
    if (chmod_error < 0) {
        perror("Failed to set file permissions");
        return -1;
    }
    return 0;
}

void close_logging() {
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}

// TODO: Need to revisit this at some point ...
int log_message(const char* format, ...) {
    if (format == NULL) {
        perror("Error text was a null pointer");
        return -1;
    }

    if (log_file == NULL) {
        perror("No open log file ...");
        return -1;
    }

    va_list args;
    va_start(args, format);

    size_t size = vsnprintf(NULL, 0, format, args);
    char text[size + 1];
    snprintf(text, BUFFER_SIZE, format, args);

    va_end(args);

    time_t now = time(NULL);
    size = 0;
    size = snprintf(NULL, 0, "[%s] %s", ctime(&now), text);
    char buffer[size + 1];
    snprintf(buffer, BUFFER_SIZE, "[%s] %s", ctime(&now), text);

    flockfile(log_file);
    fprintf(log_file, "%s\n", buffer);
    fflush(log_file);
    funlockfile(log_file);
    return 0;
}
