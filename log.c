#ifndef LOGGER_C
#define LOGGER_C

#include <stdlib.h>

typedef struct {
    unsigned max_log_len;
    unsigned max_logs;
    unsigned total_logs;
    unsigned idx;
    char** logs;
} logger_t;


// TODO a way to reset the logger/free its members

logger_t* init_logger(unsigned max_logs, unsigned max_log_len)
{
    char** logs = malloc(sizeof(char*) * max_logs);
    logger_t* retval = malloc(sizeof(logger_t));

    *retval = (logger_t) {
        .max_log_len = max_log_len,
        .max_logs = max_logs,
        .total_logs = 0,
        .idx = 0,
        .logs = logs
    };
    return retval;
}

void append_log(logger_t* logger, char* log, unsigned log_len)
{
    char* log_actual = malloc(sizeof(char) * (log_len + 1));
    memcpy(log_actual, log, log_len);
    log_actual[log_len] = 0;
    if (logger->total_logs < logger->max_logs) {
        logger->logs[logger->idx] = log_actual;
        logger->total_logs++;
        logger->idx = (logger->idx + 1) % logger->max_logs;
    } else {
        free(logger->logs[logger->idx]);
        logger->logs[logger->idx] = log_actual;

        logger->idx = (logger->idx + 1) % logger->max_logs;
    }
}

// doesn't handle the case where a single word is longer than max_log_len
void log_msg(logger_t* logger, char* format)
{
    char buf[logger->max_log_len];

    unsigned format_index = 0;
    unsigned format_len = strlen(format);

    unsigned buf_idx = 0;

    while (format_index < format_len) {
        char current_char = format[format_index];
        if (current_char == ' ') {
            // we have hit a space, peek forwards to see if we have room in `buf`
            unsigned lookahead_index = format_index + 1;
            while (format[lookahead_index] != ' ' && format[lookahead_index] != 0) {
                lookahead_index++;
            }
            if (lookahead_index - format_index <= logger->max_log_len - buf_idx) {
                buf[buf_idx] = current_char;
                format_index++;
                buf_idx++;
            } else {
                // Need to take a newline
                append_log(logger, buf, buf_idx);
                buf_idx = 0;
                format_index++;
            }
        } else {
            buf[buf_idx] = current_char;
            buf_idx++;
            format_index++;
        }
    }
    if (buf_idx != 0) {
        append_log(logger, buf, buf_idx);
    }
}

#endif
