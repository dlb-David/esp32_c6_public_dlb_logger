#include "dlb_logger.h"

dlb_logger::dlb_logger() {
}

void dlb_logger::P_up(int P) {
  if (P==1) P1++;
  if (P==2) P2++;
  if (P==3) P3++;
}

void dlb_logger::P_down(int P) {
  if (P==1) { if(P1>0) P1--; }
  if (P==2) { if(P2>0) P2--; }
  if (P==3) { if(P3>0) P3--; }
}
