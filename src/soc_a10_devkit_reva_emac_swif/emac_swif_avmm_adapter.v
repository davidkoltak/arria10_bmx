/*
  A module to control EMAC Switch Interface transations from an Avalon MM 32-bit master
  
  by David M. Koltak  03/22/2016
  
  Address     :  Register
--------------------------------------------------------
        00000 : Tx CSR
                   [0] - Busy
                   [1] - Error
                [11:2] - Last Tx Word
               [13:12] - Last Tx BE
               [28:14] - (reserved)
                  [29] - tx_chksum
                  [30] - tx_dispad
                  [31] - tx_discrc  
                                
  00004-00FFF : Tx Buffer
  
        01000 : Rx CSR
                   [0] - Valid
                   [1] - Error
                [11:2] - Last Rx Word
               [13:12] - Last Rx BE
               [31:14] - (reserved)
               
  01004-01FFF : Rx Buffer
  
*/
      
module emac_swif_avmm_adapter
(
  input clk,

  input lw_h2f_write,
  input lw_h2f_read,
  input [19:0] lw_h2f_address,
  input [3:0] lw_h2f_byteenable,
  input [31:0] lw_h2f_writedata,  
  output lw_h2f_waitrequest,
  output [31:0] lw_h2f_readdata,
  output lw_h2f_readdatavalid,
  input lw_h2f_burstcount,
  input lw_h2f_debugaccess,
  
  output switch_ati_val,
  input switch_ati_rdy,
  output switch_ati_ack,
  output [31:0] switch_ati_data,
  output [1:0] switch_ati_be,

  output switch_ati_sof,
  output switch_ati_eof,
  input switch_ati_txstatus_val,
  input [17:0] switch_ati_txstatus,
  output [8:0] switch_ati_pbl,
  input switch_ati_tx_watermark,
  output switch_ati_discrs,
  output switch_ati_dispad,
  output [1:0] switch_ati_chksum_ctrl,
  output switch_ati_ena_timestamp,
  input [63:0] switch_ati_timestamp,

  input switch_ari_val,
  output switch_ari_ack,
  input [31:0] switch_ari_data,
  input [1:0] switch_ari_be,

  input switch_ari_sof,
  input switch_ari_eof,
  input switch_ari_rxstatus_val,
  output [8:0] switch_ari_pbl,
  input switch_ari_rx_watermark,
  output switch_ari_frameflush,
  input switch_ari_timestamp_val
);

  assign lw_h2f_waitrequest = 1'b0;
  
  //
  // Transmit buffer
  //
  
  reg [31:0] tx_buf[1023:0];
  wire [31:0] bit_mask;
  
  assign bit_mask = {{8{lw_h2f_byteenable[3]}}, {8{lw_h2f_byteenable[2]}}, {8{lw_h2f_byteenable[1]}}, {8{lw_h2f_byteenable[0]}}};
  
  always @ (posedge clk)
    if (lw_h2f_write && (lw_h2f_address[19:12] == 8'd0))
      tx_buf[lw_h2f_address[11:2]] <= (tx_buf[lw_h2f_address[11:2]] & ~bit_mask) | (lw_h2f_writedata & bit_mask);

  //
  // Transmit state machine
  //
  
  reg tx_active;
  reg tx_sof;
  reg tx_eof;
  reg [9:0] tx_last;
  reg [1:0] tx_last_be;
  reg tx_discrc;
  reg tx_dispad;
  reg tx_chksum;
  reg [9:0] tx_word;
  reg [31:0] tx_data;
  
  always @ (posedge clk or posedge rst)
    if (rst)
    begin
      tx_active <= 1'b0;
      tx_sof <= 1'b0;
      tx_eof <= 1'b0;
      tx_last <= 10'd0;
      tx_last_be <= 2'b00;
      tx_discrc <= 1'b0;
      tx_dispad <= 1'b0;
      tx_chksum <= 1'b0;
      tx_word <= 10'd0;
      tx_data <= 32'd0;
    end
    else if (lw_h2f_write && (lw_h2f_address == 20'd0) && (lw_h2f_byteenable == 4'b1111))
    begin
      tx_active <= 1'b1;
      tx_sof <= 1'b1;
      tx_eof <= 1'b0;
      tx_last <= lw_h2f_writedata[11:2];
      tx_last_be <= lw_h2f_writedata[13:12];
      tx_discrc <= lw_h2f_writedata[31];
      tx_dispad <= lw_h2f_writedata[30];
      tx_chksum <= lw_h2f_writedata[29];
      tx_word <= 10'd1;
      tx_data <= tx_buf[10'd1];
    end
    else if (tx_active && switch_ati_rdy)
    begin
      tx_active <= (tx_word != tx_last);
      tx_sof <= 1'b0;
      tx_eof <= (tx_word == tx_last);
      tx_word <= lw_h2f_writedata[9:0] + 10'd1;
      tx_data <= tx_buf[tx_word];
    end
    
  //
  // Transmit
  //
  
  assign switch_ati_val = tx_active;
  assign switch_ati_data = tx_data;
  assign switch_ati_be = tx_last_be;
  
  assign switch_ati_sof = tx_sof;
  assign switch_ati_eof = tx_eof;
  
  assign switch_ati_discrs = tx_discrc;
  assign switch_ati_dispad = tx_dispad;
  assign switch_ati_chksum_ctrl = tx_chksum;
  
  reg [31:0] tx_status;
  assign switch_ati_ack = 1'b1;
  
  always @ (posedge clk)
    if (switch_ati_txstatus_val)
      tx_status = {14'd0, switch_ati_txstatus};
  
  assign switch_ati_ena_timestamp = 1'b0; // NOTE: 'switch_ati_timestamp' is unused
  assign switch_ati_pbl = 9'd0; // NOTE: 'switch_ati_tx_watermark' is unused
  
  //
  // Receive state machine
  //
  
  reg rx_active;
  reg rx_valid;
  reg [9:0] rx_word;
  
  always (posedge clk or posedge rst)
    if (rst)
    begin
      rx_active <= 1'b0;
      rx_valid <= 1'b0;
      rx_word <= 10'd0;
    end
    else if (!rx_active && !rx_valid)
    begin
    
    end
  
  //
  // Receive buffer
  //
  
  reg [31:0] rx_buf[1023:0];
  
  always @ (posedge clk or posedge rst)
    if (rx_active && switch_ari_val)
      rx_buf[rx_word] <= switch_ari_data;
  
  //
  // Receive
  //
  
  assign fifo_rx_in_valid = switch_ari_val;

  assign fifo_rx_in_data = switch_ari_data;
  assign fifo_rx_in_empty = ~switch_ari_be;
  
  assign fifo_rx_in_startofpacket = switch_ari_sof;
  assign fifo_rx_in_endofpacket = switch_ari_eof;
   
  reg [31:0] rx_status;
  assign switch_ari_ack = switch_ari_rxstatus_val || rx_active;
  
  always @ (posedge clk)
    if (switch_ari_rxstatus_val)
      rx_status = switch_ari_data;
  
  assign switch_ari_pbl = 9'd0; // NOTE: 'switch_ari_rx_watermark' is unused
  assign switch_ari_frameflush = 1'b0;
  // NOTE: 'switch_ari_timestamp_val' is unused
   
endmodule
