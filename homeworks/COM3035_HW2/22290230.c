#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define SEM_ID "/sys_ops_sem_v1"

typedef struct
{
    char id[64];
    int w, d, l;
    int gf, ga;
    int pts;
    int gd;
} team_t;

sem_t *g_sem;
int g_max = INT_MIN;
int g_min = INT_MAX;

void update_extremes(int val)
{
    sem_wait(g_sem);
    if (val > g_max)
        g_max = val;
    if (val < g_min)
        g_min = val;
    sem_post(g_sem);
}

void *worker(void *arg)
{
    team_t *t = (team_t *)arg;
    t->pts = (t->w * 3) + t->d;
    t->gd = t->gf - t->ga;
    update_extremes(t->pts);
    return NULL;
}

int sort_desc(const void *a, const void *b)
{
    const team_t *t1 = (const team_t *)a;
    const team_t *t2 = (const team_t *)b;

    if (t2->pts != t1->pts)
        return t2->pts - t1->pts;
    if (t2->gd != t1->gd)
        return t2->gd - t1->gd;
    return t2->gf - t1->gf;
}

void render_table(team_t *arr, int n, int top, int bot)
{
    int i;
    for (i = 0; i < n; i++)
    {
        printf("%s %d", arr[i].id, arr[i].pts);

        if (i < top)
        {
            printf(" - Qualified");
        }
        else if (i >= n - bot)
        {
            printf(" - Relegated");
        }
        printf("\n");
    }
    printf("\nHighest point: %d\nLowest point: %d\n", g_max, g_min);
}

int main(void)
{
    int n, n_qual, n_rel;
    int i;
    team_t *teams;
    pthread_t *tids;

    if (scanf("%d %d %d", &n, &n_qual, &n_rel) != 3)
        return 1;
    if (n <= 0)
        return 0;

    teams = (team_t *)malloc(sizeof(team_t) * n);
    tids = (pthread_t *)malloc(sizeof(pthread_t) * n);

    if (!teams || !tids)
        return 1;

    sem_unlink(SEM_ID);
    g_sem = sem_open(SEM_ID, O_CREAT, 0644, 1);

    if (g_sem == SEM_FAILED)
    {
        free(teams);
        free(tids);
        return 1;
    }

    for (i = 0; i < n; i++)
    {
        if (scanf("%63s %d %d %d %d %d",
                  teams[i].id, &teams[i].w, &teams[i].d,
                  &teams[i].l, &teams[i].gf, &teams[i].ga) != 6)
            break;

        pthread_create(&tids[i], NULL, worker, (void *)&teams[i]);
    }

    for (i = 0; i < n; i++)
    {
        pthread_join(tids[i], NULL);
    }

    sem_close(g_sem);
    sem_unlink(SEM_ID);
    free(tids);

    qsort(teams, n, sizeof(team_t), sort_desc);
    render_table(teams, n, n_qual, n_rel);

    free(teams);
    return 0;
}