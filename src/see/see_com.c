#include "../../include/see_cmd.h"
#include "../../include/see_crypto.h"

see_cmd_t *cmd = (see_cmd_t *)(SMEM_BUFFER);

void see_cmd_version() {
  cmd->result[0] = SEEOS_VERSION;
  cmd->response = CMD_SUCCESS;
}

void see_cmd_wrap_key() {
  uint64_t key_data = cmd->data[0];
  if (key_data == 0) {
    cmd->response = CMD_WRONG_ARGS;
    return;
  }

  wrapped_key_t key_blob = see_wrap_key(key_data);
  cmd->result[0] = key_blob.wrapped_key1;
  cmd->result[1] = key_blob.wrapped_key2;
  cmd->response = CMD_SUCCESS;
}

void see_cmd_unwrap_key() {
  uint64_t wrapped_key1 = cmd->data[0];
  uint64_t wrapped_key2 = cmd->data[1];
  if (wrapped_key1 == 0 || wrapped_key2 == 0) {
    cmd->response = CMD_WRONG_ARGS;
    return;
  }

  wrapped_key_t key_blob = {wrapped_key1, wrapped_key2};
  cmd->result[0] = see_unwrap_key(key_blob);
  cmd->response = CMD_SUCCESS;
}

static see_cmd_entry see_cmd[3] = {{SEE_CMD_VERSION, see_cmd_version},
                                   {SEE_CMD_WRAP_KEY, see_cmd_wrap_key},
                                   {SEE_CMD_UNWRAP_KEY, see_cmd_unwrap_key}};

void see_cmd_invoke() {
  cmd->response = CMD_UNDEFINED;
  for (int i = 0; i < 3; i++) {
    if (cmd->command == see_cmd[i].command_id) {
      see_cmd[i].see_cmd_fun();
      asm volatile("dsb ishst");
      break;
    }
  }
}
