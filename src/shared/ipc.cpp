#include <iostream>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <linux/limits.h>

#include "ipc.h"


namespace swtfb {
  unsigned int WIDTH = 1404;
  unsigned int HEIGHT = 1872;

  inline void reset_dirty(mxcfb_rect &dirty_area) {
    dirty_area.left = WIDTH;
    dirty_area.top = HEIGHT;
    dirty_area.width = 0;
    dirty_area.height = 0;
  }

  inline void mark_dirty(mxcfb_rect &dirty_area, mxcfb_rect &rect) {
    uint32_t x1 = dirty_area.left + dirty_area.width;
    uint32_t y1 = dirty_area.top + dirty_area.height;

    x1 = std::max(x1, rect.left + rect.width);
    y1 = std::max(y1, rect.top + rect.height);

    if (x1 > WIDTH) {
      x1 = WIDTH - 1;
    }
    if (y1 > HEIGHT) {
      y1 = HEIGHT - 1;
    }

    dirty_area.left = std::min(rect.left, dirty_area.left);
    dirty_area.top = std::min(rect.top, dirty_area.top);

    dirty_area.width = x1 - dirty_area.left;
    dirty_area.height = y1 - dirty_area.top;
  }

  namespace ipc {

    int BUF_SIZE = maxWidth * maxHeight *
      sizeof(uint16_t); // hardcoded size of display mem for rM2
    string DEFAULT_NAME= "/swtfb.01";
    int SWTFB_FD = 0;

    // TODO: allow multiple shared buffers in one process?
    uint16_t *get_shared_buffer(string name = DEFAULT_NAME, char * imgBuffer = NULL) {
      mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
      if (name[0] != '/') {
        name = "/" + name;
      }
      const char* cname = name.c_str();

      int fd = shm_open(cname, O_RDWR | O_CREAT, mode);
      if (fd == -1) {
        fprintf(stderr, "CANNOT OPEN shared mem: %s,  %i, %s\n", cname, errno, strerror(errno));
        return nullptr;
      }


      fprintf(stderr, "SHM FD: %i \n", fd);
      SWTFB_FD = fd;

      ftruncate(fd, BUF_SIZE);
      uint16_t *mem =
        (uint16_t *)mmap(imgBuffer, BUF_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
      fprintf(stderr, "OPENED SHARED MEM: /dev/shm%s at %x, errno: %i\n", name.c_str(), mem, errno);
      return mem;
    }

  }; // namespace ipc
}; // namespace swtfb
