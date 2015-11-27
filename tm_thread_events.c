#include "tm_thread.h"

#include "tm_alloc.h"
#include "tm_configuration.h"
#include "tm_time.h"
#include "tm_logging.h"

#include <ev.h>
#include <pthread.h>
#include <fcntl.h>

static int signals[] = {
		SIGINT,
		SIGTERM,
		SIGQUIT,
		SIGTSTP,
		SIGPWR
};

#define SIGNAL_COUNT (sizeof(signals) / sizeof(signal))

typedef struct {
	pthread_t thread_id;

	struct ev_loop *loop;
	ev_async w_shutdown;
	ev_io w_pipe_read;
	int pipe_fd[2];

	ev_io w_pipe_write;
	ev_timer w_next_io_start;
	ev_async w_low_bitrate;

	tm_time_bitrate *bitrate;
	size_t clients_count;
} tm_work_thread_t;

typedef struct {
	struct ev_loop *loop;
	ev_timer w_test_time;
	ev_signal w_signals[SIGNAL_COUNT];

	int low_bitrate_flag;

	tm_work_thread_t *work_threads;
	size_t work_threads_count;

} tm_main_thread_t;

static tm_main_thread_t main_thread;

/**
 * Коллбэк для ev_io wathcer'а, который используется для записи в пайпы (по одному на рабочий поток).
 * Крутятся на петле главного потока. Вызывается, когда главный поток может писать в пайп.
 */
static void pipe_write_callback(EV_P_ ev_io *w, int revents)
{
	//TODO
}

/**
 * Коллбэк для ev_timer watcher'a, который используется для запуска watcher'а на запись в пайп.
 * По одному на каждый рабочий поток. Крутятся на петле главного потока. Запускаются, когда мы достигли
 * нужной скорости, чтобы подождать до конца отсчётного периода битрейта.
 */
static void timer_for_next_io_callback(EV_P_ ev_timer *w, int revents)
{
	//TODO
}

/**
 * Коллбэк для ev_async wathcer'а, который используется для сигнализации главному потоку о низком битрейте.
 * По одному на рабочий поток. Крутятся на петле главного потока. Посылают рабочие потоки.
 */
static void low_bitrate_callback(EV_P_ ev_async *w, int revents)
{
	//TODO
}

/**
 * Функция рабочего потока
 */
static void *work_thread_function(void *arg)
{
	//TODO
}

/**
 * Коллбэк для ev_async wathcer'а, который используется для завершения потока
 * Сидит на петле рабочего потока. Посылает главный поток.
 */
static void thread_shutdown_callback(EV_P_ ev_async *w, int revents)
{
	//TODO
}

/**
 * Коллбэк для ev_io watcher'а, который используется для чтения из пайпа (по одному на рабочий поток).
 * Крутятся на петлях рабочих потоков. Вызываются, когда можем прочитать посланные
 * главным потоком данные.
 */
static void pipe_read_callback(EV_P_ ev_io *w, int revents)
{
	//TODO
}

static TMThreadStatus tm_thread_thread_create(tm_work_thread_t *thread)
{
	TMThreadStatus status = TMThreadStatus_ERROR;

	if (!thread)
		return status;

	if (thread->clients_count <= 0) {
		TM_LOG_ERROR("Attempt to start thread with <= 0 clients on it.");
		return status;
	}

	thread->bitrate = tm_calloc(thread->clients_count * sizeof(tm_time_bitrate));
	if (!thread->bitrate) {
		TM_LOG_ERROR("Failed to allocate memory for bitrate contexts.");
		return status;
	}

	pipe(thread->pipe_fd);
	fcntl(thread->pipe_fd[0], F_SETFD, O_NONBLOCK);
	fcntl(thread->pipe_fd[1], F_SETFD, O_NONBLOCK);

	thread->loop = ev_loop_new(0);
	if (!thread->loop) {
		TM_LOG_ERROR("Failed to create loop for work thread.");
		return status;
	}

	ev_async_init(&thread->w_shutdown, thread_shutdown_callback);
	ev_async_start(thread->loop, &thread->w_shutdown);

	ev_io_init(&thread->w_pipe_read, pipe_read_callback, thread->pipe_fd[0], EV_READ);
	ev_io_start(thread->loop, &thread->w_pipe_read);

	status = TMThreadStatus_SUCCESS;
	return status;
}

static void tm_thread_shutdown(tm_work_thread_t *thread)
{
	if (thread) {

	}
}

static void signal_callback(EV_P_ ev_signal *w, int revents)
{
	//TODO

	for (int i = 0; i < SIGNAL_COUNT; i++) {
		ev_signal_stop(main_thread.loop, &main_thread.w_signals[i]);
	}
}

static void test_time_callback(EV_P_ ev_timer *w, int revents)
{
	//TODO
}

static TMThreadStatus tm_threads_init(int count)
{
	TMThreadStatus status = TMThreadStatus_ERROR;
	size_t clients_count;

	if (count <= 0) {
		TM_LOG_ERROR("Attempt to create <= 0 work threads.");
		return status;
	}

	main_thread.work_threads = tm_calloc(count * sizeof(tm_work_thread_t));
	if (!main_thread.work_threads) {
		TM_LOG_ERROR("Failed to allocate memory.");
		return status;
	}

	main_thread.loop = EV_DEFAULT;
	ev_timer_init(&main_thread.w_test_time, test_time_callback, (double) configuration.test_time, 0.0);

	for (int i = 0; i < SIGNAL_COUNT; i++) {
		ev_signal_init(&main_thread.w_signals[i], signal_callback, signals[i]);
		ev_signal_start(main_thread.loop, &main_thread.w_signals[i]);
	}

	clients_count = (size_t) configuration.clients_count / count;
	for(int i = 0; i < count; i++) {
		main_thread.work_threads[i].clients_count = clients_count;
	}
	for(int i = 0; i < configuration.clients_count % count; i++) {
		main_thread.work_threads[i].clients_count++;
	}

	main_thread.work_threads_count = 0;
	for(int i = 0; i < main_thread.work_threads_count; i++) {
		if (tm_thread_thread_create(&main_thread.work_threads[i]) != TMThreadStatus_SUCCESS) {
			TM_LOG_ERROR("Failed to create a work thread.");
			return status;
		}
		main_thread.work_threads_count++;
	}

	status = TMThreadStatus_SUCCESS;
	return status;
}
