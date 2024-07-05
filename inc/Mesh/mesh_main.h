#ifndef MESH_MAIN_H
#define MESH_MAIN_H

#include "../Custom/main.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* GWY GLOBAL VARIABLES */

/* NODE GLOBAL VARIABLES */

/* GWY FUNCTION DECLARATIONS */
void gwy_mesh_main_init(void);
void send_prov_packet_to_node(prov_t *prov_packet);
void send_unprov_packet_to_node(unprov_t *unprov_packet);
void send_reconf_packet_to_node(reconf_t *reconf_packet);
void send_ac_control_packet_to_node(control_t *control_packet);
void send_pub_conf_packet_to_node(pub_conf_t *pub_conf_packet);
void send_teaching_mode_packet_to_node(teaching_mode_t *node_teaching_mode_packet);
void send_debug_info_packet_to_node(debug_info_t *debug_info_packet);
void zero_out_match_arr_in_mesh();

/* NODE FUNCTION DECLARATIONS */
void node_mesh_main_init(void);
void send_AC_configuration_ack_to_gwy();
void send_provisioned_ack_to_gwy();
void send_unprovisioned_ack_to_gwy();
void send_node_ac_control_ack_to_gwy();
void send_manual_ac_control_ack_to_gwy();
void send_heartbeat_ack_to_gwy();
void send_heartbeat_publish_configuration_ack_to_gwy();
void send_teaching_mode_start_ack_to_gwy();
void send_teaching_mode_end_ack_to_gwy();
void fill_element_addr_to_all_structures();
void fetch_debug_info();

#ifdef __cplusplus
}
#endif

#endif
