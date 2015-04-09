
#include "tm_queue.h"
#include "tm_read_config.h"
#include "tm_alloc.h"
#include <stdio.h>


tm_queue_t* tm_queue_create()
{
	tm_queue_t *q = tm_alloc(sizeof(tm_queue_t));

	if(!q)
		return q;

	pthread_mutex_init(&q->mutex, NULL);
	pthread_cond_init(&q->cond, NULL);
	q->buffers = tm_calloc(QUEUE_SIZE * sizeof(void*));
	q->size = 0;
	q->in = 0;
	q->out = 0;

	return q;
}

void tm_queue_destroy(tm_queue_t *q)
{
	pthread_mutex_destroy(&q->mutex);
	pthread_cond_destroy(&q->cond);
	while (q->size > 0)
	{
		tm_free(q->buffers[q->out]);
		q->buffers[q->out] = NULL;
		q->size--;
		q->out++;
		q->out %= QUEUE_SIZE;
	}
	tm_free(q->buffers);
	tm_free(q);
}

/*
 * Добавить очередной блок данных в конец.
 * Это действие производит только главный поток.
 * Лочится мьютекс, если очередь полна, то блок данных
 * как бы теряется и возвращается 0.
 * Если не полна, то делается calloc(так добавляется новый блок)
 * увеличивается указатель записи, размер, посылается сигнал, чтобы
 * разбудить, если рабочий поток спит на мьютексе и разлачивается мьюеткс.
*/
int tm_queue_push_back(tm_queue_t *q)
{
	pthread_mutex_lock(&q->mutex);
	if (q->size == QUEUE_SIZE)
	{
		pthread_cond_signal(&q->cond);
		pthread_mutex_unlock(&q->mutex);
		return 0;
	}

	void *buf = (void*)tm_calloc(configuration.block_size);

	q->buffers[q->in] = buf;
	printf("block added to queue %lu\n", (unsigned long)q);
	q->size++;
	q->in++;
	q->in %= QUEUE_SIZE;

	pthread_cond_signal(&q->cond);
	pthread_mutex_unlock(&q->mutex);
	return 1;
}

/*
 * Забрать элемент из очереди. Производит только рабочий поток.
 * Лочится мьютекс и, если очередь пуста, то засыпаем на кондишоне.
 * Если не пуста, то копируем блок из очереди и освобождаем.
 * Уменьшаем размер, увеличиваем указатель чтения. Разлачиваем мьютекс.
 * Возвращаем указатель на скопированный блок.
 * Рабочий поток должен его потом освободить.
*/

void* tm_queue_pop_front(tm_queue_t *q)
{
	void* res = NULL;

	pthread_mutex_lock(&q->mutex);
	while(q->size == 0)
		pthread_cond_wait(&q->cond, &q->mutex);

	res = tm_strdup(q->buffers[q->out], configuration.block_size);
	tm_free(q->buffers[q->out]);
	q->buffers[q->out] = NULL;
	printf("block removed from queue %lu\n", (unsigned long)q);
	q->size--;
	q->out++;
	q->out %= QUEUE_SIZE;
	pthread_mutex_unlock(&q->mutex);
	return res;
}
