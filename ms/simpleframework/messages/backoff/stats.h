#ifndef __MESSAGES_BACKOFF_STATS_H__
#define __MESSAGES_BACKOFF_STATS_H__

typedef struct BackoffStats
{
    uint16_t sent[SETTINGS_MAX_SENSORS];
    uint16_t received[SETTINGS_MAX_SENSORS];
    uint16_t ack[SETTINGS_MAX_SENSORS][SETTINGS_BACKOFF_TRIES_LIMIT];
} BackoffStats;

extern BackoffStats backoff_stats;

void show_backoff_stats(Time_cptr *time, BackoffStats *stats);

#endif // __MESSAGES_BACKOFF_STATS_H__
