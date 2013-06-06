#ifndef LIBCIRCLE_H
#define LIBCIRCLE_H

/* define a C interface */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * The maximum length of a string value which is allowed to be placed on the
 * queue structure.
 */
#ifdef PATH_MAX
    #define CIRCLE_MAX_STRING_LEN PATH_MAX
#else
    #define CIRCLE_MAX_STRING_LEN (4096)
#endif

/**
 * Run time flags for the behavior of splitting work.
 */
#define CIRCLE_SPLIT_RANDOM     (1 << 0)              /* Split work randomly. */
#define CIRCLE_SPLIT_EQUAL      ~CIRCLE_SPLIT_RANDOM  /* Split work evenly */
#define CIRCLE_DEFAULT_FLAGS    CIRCLE_SPLIT_RANDOM   /* Default behavior is random work stealing */

/**
 * The various logging levels that libcircle will output.
 */
typedef enum CIRCLE_loglevel
{
    CIRCLE_LOG_FATAL = 1,
    CIRCLE_LOG_ERR   = 2,
    CIRCLE_LOG_WARN  = 3,
    CIRCLE_LOG_INFO  = 4,
    CIRCLE_LOG_DBG   = 5
} CIRCLE_loglevel;

/**
 * The interface to the work queue. This can be accessed from within the
 * process and create work callbacks. The type of element must be a NULL
 * terminated string.
 */
typedef struct {
    int8_t (*enqueue)(char *element);
    int8_t (*dequeue)(char *element);
    uint32_t (*local_queue_size)(void);
} CIRCLE_handle;

/**
 * The type for defining callbacks for create and process.
 */
typedef void (*CIRCLE_cb)(CIRCLE_handle *handle);

/**
 * Initialize internal state needed by libcircle. This should be called before
 * any other libcircle API call. This returns the MPI rank value.
 */
int CIRCLE_init(int argc, char *argv[], int options);

/**
 * Change run time flags
 */
void CIRCLE_set_options(int options);

/**
 * Processing and creating work is done through callbacks. Here's how we tell
 * libcircle about our function which creates work. This call is optional.
 */
void CIRCLE_cb_create(CIRCLE_cb func);

/**
 * After you give libcircle a way to create work, you need to tell it how that
 * work should be processed.
 */
void CIRCLE_cb_process(CIRCLE_cb func);

/**
 * Once you've defined and told libcircle about your callbacks, use this to
 * execute your program.
 */
void CIRCLE_begin(void);

/**
 * Call this function to have all ranks dump a checkpoint file and exit.
 */
void CIRCLE_abort(void);

/**
 * Call this function to checkpoint libcircle's distributed queue. Each rank
 * writes a file called circle<rank>.txt
 */
void CIRCLE_checkpoint(void);

/**
  * Function to return a pointer to the handle.  Useful for threaded applications.
  * You are responsible for maintaining mutual exclusion.
  */
CIRCLE_handle* CIRCLE_get_handle(void);

/**
 * Call this function to initialize libcircle queues from restart files
 * created by CIRCLE_checkpoint.
 */
void CIRCLE_read_restarts(void);

/**
 * After your program has executed, give libcircle a chance to clean up after
 * itself by calling this. This should be called after all libcircle API calls.
 */
void CIRCLE_finalize(void);

/**
 * Define the detail of logging that libcircle should output.
 */
void CIRCLE_enable_logging(enum CIRCLE_loglevel level);

/**
 * Returns an elapsed time on the calling processor for benchmarking purposes.
 */
double CIRCLE_wtime(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIBCIRCLE_H */
