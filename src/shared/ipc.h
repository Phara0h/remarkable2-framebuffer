#pragma once
#include "mxcfb.h"
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <string>

namespace swtfb {


  extern unsigned int WIDTH;
  extern unsigned int HEIGHT;

  struct swtfb_update {
    long mtype;
    struct mxcfb_update_data update;
#ifdef DEBUG_TIMING
    uint64_t ms;
#endif
  };

  namespace ipc {

    using namespace std;
    enum MSG_TYPE { INIT_t = 1, UPDATE_t };

    extern string DEFAULT_NAME;

    extern "C" {
      uint16_t *get_shared_buffer(std::string name, char * );
    }
    const int maxWidth = 1404;
    const int maxHeight = 1872;
    extern int SWTFB_FD;
    extern int BUF_SIZE;

#define SWTFB1_UPDATE 1
    class Queue {
      public:
        unsigned long id;
        int msqid = -1;

        void init() { msqid = msgget(id, IPC_CREAT | 0600); }

        Queue(int id) : id(id) { init(); }

        void send(mxcfb_update_data msg) {
          // TODO: read return value
#ifdef DEBUG
          auto rect = msg.update_region;
          cerr << get_now() << " MSG Q SEND " << rect.left << " " << rect.top << " "
            << rect.width << " " << rect.height << endl;
#endif

          swtfb_update swtfb_msg;
          swtfb_msg.mtype = 1;
          swtfb_msg.update = msg;

#ifdef DEBUG_TIMING
          swtfb_msg.ms = get_now();
#endif
          int wrote = msgsnd(msqid, (void *)&swtfb_msg, sizeof(swtfb_msg), 0);
          if (wrote != 0) {
            cerr << "ERRNO " << errno << endl;
          }
        }

        swtfb_update recv() {
          swtfb_update buf;
          auto len = msgrcv(msqid, &buf, sizeof(buf), 0, 0);
#ifdef DEBUG_TIMING
          auto rect = buf.update.update_region;
          cerr << get_now()  - buf.ms << "ms MSG Q RECV " << rect.left << " " << rect.top << " "
            << rect.width << " " << rect.height << endl;
#endif
          if (len >= 0) {
            return buf;
          } else {
            cerr << "ERR " << len << " " << errno << endl;
          }

          return {};
        }

        void destroy() { msgctl(msqid, IPC_RMID, 0); };
    };
  }
}
