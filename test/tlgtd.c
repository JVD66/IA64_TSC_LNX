#include "linux_gtod_page.h"

int main(int argc, char **argv, char **envp)
{ if( _lnx_gtod.vclock_mode == 1 )
  { printf("it worked! - GTOD: clock:%u mult:%u shift:%u\n", _lnx_gtod.vclock_mode, _lnx_gtod.mult, _lnx_gtod.shift);
    sleep(1);
    lnx_gtod_sync();
    printf("synced - mult now: %u\n",_lnx_gtod.mult);
  }
}
