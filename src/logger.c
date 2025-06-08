#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h>
#include <sys/stat.h>

static FILE *log_file = NULL;

int init_logging(const char *filename) {
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

int log_message(const char *format, ...) {
    if (format == NULL) {
        perror("Error text was a null pointer");
        return -1;
    }

    if (log_file == NULL) {
        perror("No open log file ...");
        return -1;
    }

    va_list args1;
    va_start(args1, format);
    va_list args2;
    va_copy(args2, args1);

    size_t size = vsnprintf(NULL, 0, format, args1);
    va_end(args1);

    char text[size + 1];
    vsnprintf(text, sizeof(text), format, args2);

    va_end(args2);

    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    char time_buf[100];
    strftime(
        time_buf,
        sizeof(time_buf),
        "%D %T",
        gmtime(&ts.tv_sec)
    );

    size = snprintf(NULL, 0, "%s: %s", time_buf, text);
    char buffer[size + 1];
    snprintf(buffer, sizeof(buffer), "%s: %s", time_buf, text);

    flockfile(log_file);
    fprintf(log_file, "%s\n", buffer);
    fflush(log_file);
    funlockfile(log_file);

    return 0;
}
