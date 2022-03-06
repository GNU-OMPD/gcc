#include "libgomp.h"
#include "ompd-support.h"
#include <limits.h>

ompd_rc_t ompd_get_curr_parallel_handle(
    ompd_thread_handle_t *thread_handle,     /* IN: OpenMP thread handle*/
    ompd_parallel_handle_t **parallel_handle /* OUT: OpenMP parallel handle */
    ) {
  if (!thread_handle)
    return ompd_rc_stale_handle;
  if (!thread_handle->ah)
    return ompd_rc_stale_handle;
  ompd_address_space_context_t *context = thread_handle->ah->context;
  ompd_thread_context_t *thread_context = thread_handle->thread_context;
  if (!context || !thread_context)
    return ompd_rc_stale_handle;

  if (!callbacks) {
    return ompd_rc_callback_error;
  }

  ompd_rc_t ret;

  if (thread_handle->ah->kind == OMPD_DEVICE_KIND_CUDA) {
    ompd_address_t taddr;
    TValue ph;
    // The ompd_parallel_info_t we need is only present in the previous task
    // of an implicit task.
    uint16_t task_is_implicit = 0;
    ret = ompd_rc_ok;
    auto possibleTaskDescr = TValue(context, thread_handle->th)
                              .cast("omptarget_nvptx_TaskDescr", 0,
                                     OMPD_SEGMENT_CUDA_PTX_GLOBAL);

    while (!task_is_implicit && ret == ompd_rc_ok) {
      ret = possibleTaskDescr.access("ompd_thread_info")
                             .cast("ompd_nvptx_thread_info_t", 0,
                                   OMPD_SEGMENT_CUDA_PTX_GLOBAL)
                             .access("task_implicit")
                             .castBase()
                             .getValue(task_is_implicit);
      possibleTaskDescr = possibleTaskDescr.access("prev")
                                           .cast("omptarget_nvptx_TaskDescr",
                                                 1, OMPD_SEGMENT_CUDA_PTX_GLOBAL);
      ret = possibleTaskDescr.dereference().getAddress(&taddr);
    }

    if (ret != ompd_rc_ok) {
      if (taddr.address == 0) {
        ph = TValue(context, NULL, "omptarget_nvptx_threadPrivateContext")
                 .cast("omptarget_nvptx_ThreadPrivateContext", 1,
                       OMPD_SEGMENT_CUDA_PTX_SHARED)
                  .access("ompd_levelZeroParallelInfo")
                  .cast("ompd_nvptx_parallel_info_t", 0,
                        OMPD_SEGMENT_CUDA_PTX_GLOBAL);
      } else {
        return ret;
      }
    } else {
      ph = possibleTaskDescr.access("ompd_thread_info")
               .cast("ompd_nvptx_thread_info_t", 0,
                      OMPD_SEGMENT_CUDA_PTX_GLOBAL)
               .access("enclosed_parallel")
               .cast("ompd_nvptx_parallel_info_t", 0,
                     OMPD_SEGMENT_CUDA_PTX_GLOBAL);
    }

    ret = ph.getAddress(&taddr);
    if (ret != ompd_rc_ok)
      return ret;

    ret = callbacks->alloc_memory(sizeof(ompd_parallel_handle_t),
                                 (void **)(parallel_handle));
    if (ret != ompd_rc_ok)
      return ret;

    (*parallel_handle)->ah = thread_handle->ah;
    (*parallel_handle)->th = taddr;
    (*parallel_handle)->cuda_kernel_info = thread_handle->cuda_kernel_info;
  } else {
    ompd_address_t taddr={OMPD_SEGMENT_UNSPECIFIED,0}, lwt={OMPD_SEGMENT_UNSPECIFIED,0};

    TValue teamdata = TValue(context, thread_handle->th) /*__kmp_threads[t]->th*/
                          .cast("kmp_base_info_t")
                          .access("th_team") /*__kmp_threads[t]->th.th_team*/
                          .cast("kmp_team_p", 1)
                          .access("t"); /*__kmp_threads[t]->th.th_team->t*/

    ret = teamdata.getAddress(&taddr);
    if (ret != ompd_rc_ok)
      return ret;

    lwt.segment = OMPD_SEGMENT_UNSPECIFIED;
    ret = teamdata.cast("kmp_base_team_t", 0)
              .access("ompt_serialized_team_info")
              .castBase()
              .getValue(lwt.address);
    if (ret != ompd_rc_ok)
      return ret;

    ret = callbacks->alloc_memory(sizeof(ompd_parallel_handle_t),
                                   (void **)(parallel_handle));
    if (ret != ompd_rc_ok)
      return ret;

    (*parallel_handle)->ah = thread_handle->ah;
    (*parallel_handle)->th = taddr;
    (*parallel_handle)->lwt = lwt;
  }
  return ompd_rc_ok;
}

ompd_rc_t ompd_get_task_parallel_handle(
    ompd_task_handle_t *task_handle, /* IN: OpenMP task handle */
    ompd_parallel_handle_t *
        *task_parallel_handle /* OUT: OpenMP parallel handle */
    ) {
  if (!task_handle)
    return ompd_rc_stale_handle;
  if (!task_handle->ah)
    return ompd_rc_stale_handle;
  ompd_address_space_context_t *context = task_handle->ah->context;

  if (!context)
    return ompd_rc_stale_handle;

  if (!callbacks) {
    return ompd_rc_callback_error;
  }

  ompd_address_t taddr={OMPD_SEGMENT_UNSPECIFIED,0};

  ompd_rc_t ret;

  if (task_handle->ah->kind == OMPD_DEVICE_KIND_CUDA) {
    TValue parallelHandle;
    // The ompd_parallel_info_t we need is only present in the previous task
    // of an implicit task.
    uint16_t task_is_implicit = 0;
    ret = ompd_rc_ok;
    auto possibleTaskDescr = TValue(context, task_handle->th)
                              .cast("omptarget_nvptx_TaskDescr", 0,
                                     OMPD_SEGMENT_CUDA_PTX_GLOBAL);

    while (!task_is_implicit && ret == ompd_rc_ok) {
      ret = possibleTaskDescr.access("ompd_thread_info")
                             .cast("ompd_nvptx_thread_info_t", 0,
                                   OMPD_SEGMENT_CUDA_PTX_GLOBAL)
                             .access("task_implicit")
                             .castBase()
                             .getValue(task_is_implicit);
      possibleTaskDescr = possibleTaskDescr.access("prev")
                                           .cast("omptarget_nvptx_TaskDescr",
                                                 1, OMPD_SEGMENT_CUDA_PTX_GLOBAL);
      ret = possibleTaskDescr.dereference().getAddress(&taddr);
    }

    if (ret != ompd_rc_ok) {
      if (taddr.address == 0) {
        parallelHandle = TValue(context, NULL,
                                "omptarget_nvptx_threadPrivateContext")
                            .cast("omptarget_nvptx_ThreadPrivateContext", 1,
                                  OMPD_SEGMENT_CUDA_PTX_SHARED)
                            .access("ompd_levelZeroParallelInfo")
                            .cast("ompd_nvptx_parallel_info_t", 0,
                                  OMPD_SEGMENT_CUDA_PTX_GLOBAL);
      } else {
        return ret;
      }
    } else {
      parallelHandle = possibleTaskDescr.access("ompd_thread_info")
                                        .cast("ompd_nvptx_thread_info_t", 0,
                                              OMPD_SEGMENT_CUDA_PTX_GLOBAL)
                                        .access("enclosed_parallel")
                                        .cast("ompd_nvptx_parallel_info_t", 0,
                                              OMPD_SEGMENT_CUDA_PTX_GLOBAL);
    }
    ret = parallelHandle.getAddress(&taddr);
  } else {
    ret = TValue(context, task_handle->th)
              .cast("kmp_taskdata_t") /*td*/
              .access("td_team")      /*td.td_team*/
              .cast("kmp_team_p", 1)
              .access("t") /*td.td_team->t*/
              .getAddress(&taddr);
  }
  if (ret != ompd_rc_ok)
    return ret;

  ret = callbacks->alloc_memory(sizeof(ompd_parallel_handle_t),
                                 (void **)(task_parallel_handle));
  if (ret != ompd_rc_ok)
    return ret;

  (*task_parallel_handle)->ah = task_handle->ah;
  (*task_parallel_handle)->lwt = task_handle->lwt;
  (*task_parallel_handle)->th = taddr;
  (*task_parallel_handle)->cuda_kernel_info = task_handle->cuda_kernel_info;
  return ompd_rc_ok;
}

ompd_rc_t ompd_rel_parallel_handle(
    ompd_parallel_handle_t *parallel_handle /* IN: OpenMP parallel handle */
    ) {
  if (!parallel_handle)
    return ompd_rc_stale_handle;
  ompd_rc_t ret = callbacks->free_memory((void *)(parallel_handle));
  if (ret != ompd_rc_ok)
    return ret;
  return ompd_rc_ok;
}

ompd_rc_t
ompd_parallel_handle_compare(ompd_parallel_handle_t *parallel_handle_1,
                             ompd_parallel_handle_t *parallel_handle_2,
                             int *cmp_value) {
  if (!parallel_handle_1)
    return ompd_rc_stale_handle;
  if (!parallel_handle_2)
    return ompd_rc_stale_handle;
  if (!cmp_value)
    return ompd_rc_bad_input;
  if (parallel_handle_1->ah->kind != parallel_handle_2->ah->kind)
    return ompd_rc_bad_input;
  if (parallel_handle_1->ah->kind == OMPD_DEVICE_KIND_HOST) {
    if (parallel_handle_1->th.address - parallel_handle_2->th.address)
      *cmp_value = parallel_handle_1->th.address - parallel_handle_2->th.address;
    else
      *cmp_value =
          parallel_handle_1->lwt.address - parallel_handle_2->lwt.address;
  } else {
    *cmp_value = parallel_handle_1->th.address - parallel_handle_2->th.address;
  }
  return ompd_rc_ok;
}


