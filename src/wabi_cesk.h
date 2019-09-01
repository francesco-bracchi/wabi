
typedef struct wabi_cesk_apply_struct {
  wabi_val previous;
  wabi_val arguments;
} wabi_cesk_apply_t;

typedef struct wabi_cesk_call_struct {
  wabi_val previous;
  wabi_val arguments;
} wabi_cesk_call_t;


typedef struct wabi_vm_struct {
  wabi_val c;
  wabi_env e;
  wabi_store s;
  wabi_cont k;
  wabi_map m;
}
