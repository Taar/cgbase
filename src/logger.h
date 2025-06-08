#pragma once

int init_logging(const char *filename);
void close_logging();
int log_message(const char *text, ...);
