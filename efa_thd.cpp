#include "efa_thd.h"
#include <sstream>

namespace trans {

void efa_worker_thd(std::string thd_name, trans::EFAEndpoint **efa,
                    std::queue<Tasks *> *task_q, std::mutex *task_m, int* cntr) {
  *efa = new trans::EFAEndpoint(thd_name + "-efa-ep");
  struct fid_ep *ep = (*efa)->ep;
  // assume always communicate to peer 0
  // but make sure address already inserted through fi_av_insert()
  fi_addr_t peer_addr = 0;
  int total_size = 200 * 1024 * 1024; // 200MB
  int batch_p_size = 2 * 1024 * 1024; // 2MB
  char * local_p_buf = new char[total_size];
  try {
    while (1) {
      // only pop here, delete lock
      std::lock_guard<std::mutex> _lock(*(task_m));
      if (!task_q->empty()) {
        if (!(*efa)->av_ready) {
          std::cerr << "== address vector of peer is not inserted\n";
          exit(1);
        }

        Tasks *t = task_q->front();
        task_q->pop();

        auto s = time_now();
        std::cout << "== worker thd got new tasks " 
                  << s << "\n";
        for (int i = 0; i < t->numTask; ++i) {
          // size_t len = t->sizes[i];
          // void *_buf = t->bufs[i];
          char *_buf_s = local_p_buf + i * batch_p_size;
          if (t->type == SEND) {
            int err = fi_send(ep, _buf_s, batch_p_size, NULL, peer_addr, NULL);
            if (err < 0)
              std::cerr << "== fi_send Err: " << err << "\n";
          } else if (t->type == RECV) {
            int err = fi_recv(ep, _buf_s, batch_p_size, NULL, FI_ADDR_UNSPEC, NULL);
            if (err < 0)
              std::cerr << "== fi_recv Err: " << err << "\n";
          } else {
            std::cerr << "== impossible task type encoutered\n";
          }
          // double _t = time_now();
          // std::cout << i << " ** async task complete : " << _t << " \n";
        }
        auto e = time_now();
        std::cout << "== all async tasks launched " 
                  << e << "\n";
        // wait 
        if (t->type == RECV) {
          wait_cq((*efa)->rxcq, t->numTask);
        } else {
          wait_cq((*efa)->txcq, t->numTask);
        }
        (*cntr) += t->numTask;
      } else {
        // std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
    }
  } catch (...) {
    std::cout << "Raise exception Worker-operation-thread;\n";
    std::cout << "Terminating current thread\n";
    std::terminate();
  }
};
}; // namespace trans