#ifndef H_CLIENT_CALLBACKS_GUARD
#define H_CLIENT_CALLBACKS_GUARD

// Callbacks are fixed to specific arguments

// Callback definition templates
//  remote:
// 			void cb_remote_<name>(int ptype, int plen, void* payload);
//  local: 
// 			void cb_local_<name>(char *buff, int len);
//

void cb_remote_default(int ptype, int plen, void* payload);
void cb_local_default(char *buff, int len);

#endif
