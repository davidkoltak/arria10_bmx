/*
  A module to control EMAC Switch Interface packet transactions 
  from an Avalon MM 32-bit master.
  
  by David M. Koltak  03/22/2016
  
  Address     :  Register
--------------------------------------------------------

 --- TX INTERFACE ---
     
        00000 : Tx CSR
                   [0] - Active (Set to send full frame, auto clears)
                   [1] - Done (Set at completion, clear by any write to CSR)
                [11:2] - Last Tx Word
               [13:12] - Last Tx BE
               [28:14] - (reserved)
               [29:28] - tx_chksum
                  [30] - tx_dispad
                  [31] - tx_discrc 
                   
        00004 : Tx Status
                
  00008-00FFF : Tx Buffer (Write Only)
     
 --- RX INTERFACE ---
     
        01000 : Rx CSR
                   [0] - Active (Set to capture of next full frame, auto clears)
                   [1] - Done (Set at completion, clear by any write to CSR)
                [11:2] - Last Rx Word
               [13:12] - Last Rx BE
               [30:14] - (reserved)
                  [31] - Flush
        
        01004 : Rx Status
        
  01008-01FFF : Rx Buffer (Read Only)
  
*/
      
module emac_swif_avmm_adapter
(
  input clk,
  input rst,
  
  input lw_h2f_write,
  input lw_h2f_read,
  input [12:0] lw_h2f_address,
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
  
  //
  // Transmit buffer
  //
  
  reg [31:0] tx_buf[1023:0];
  
  always @ (posedge clk)
    if (lw_h2f_write && (lw_h2f_address[12] == 1'b0) && (lw_h2f_byteenable == 4'b1111))
      tx_buf[lw_h2f_address[11:2]] <= lw_h2f_writedata;

  //
  // Transmit state machine
  //
  
  reg tx_active;
  reg tx_send;
  reg tx_done;
  reg tx_sof;
  reg tx_eof;
  reg [9:0] tx_last;
  reg [1:0] tx_last_be;
  reg tx_discrc;
  reg tx_dispad;
  reg [1:0] tx_chksum;
  reg [9:0] tx_word;
  reg [31:0] tx_data;
  
  reg switch_ati_rdy_d1;
  
  always @ (posedge clk)
    switch_ati_rdy_d1 <= switch_ati_rdy;  // Do I need to do this??

  always @ (posedge clk or posedge rst)
    if (rst)
    begin
      tx_active <= 1'b0;
      tx_send <= 1'b0;
      tx_done <= 1'b0;
      tx_sof <= 1'b0;
      tx_eof <= 1'b0;
      tx_last <= 10'd0;
      tx_last_be <= 2'b00;
      tx_discrc <= 1'b0;
      tx_dispad <= 1'b0;
      tx_chksum <= 2'b00;
      tx_word <= 10'd0;
      tx_data <= 32'd0;
    end
    else if (lw_h2f_write && (lw_h2f_address == 13'h0000) && (lw_h2f_byteenable == 4'b1111))
    begin
      tx_active <= lw_h2f_writedata[0];
      tx_send <= 1'b0;
      tx_done <= 1'b0;
      tx_sof <= 1'b0;
      tx_eof <= 1'b0;
      tx_last <= lw_h2f_writedata[11:2] + 10'd2;
      tx_last_be <= lw_h2f_writedata[13:12];
      tx_discrc <= lw_h2f_writedata[31];
      tx_dispad <= lw_h2f_writedata[30];
      tx_chksum <= lw_h2f_writedata[29:28];
      tx_word <= 10'd2;
    end
    else if (tx_active && !tx_send)
    begin
      tx_send <= 1'b1;
      tx_sof <= 1'b1;
      tx_word <= tx_word + 10'd1;
      tx_data <= tx_buf[tx_word];
    end
    else if (tx_active && switch_ati_rdy_d1)
    begin
      tx_active <= !tx_eof;
      tx_send <= !tx_eof;
      tx_done <= tx_eof;
      tx_sof <= 1'b0;
      tx_eof <= (tx_word == tx_last);
      tx_word <= tx_word + 10'd1;
      tx_data <= tx_buf[tx_word];
    end
    
  //
  // Transmit
  //
  
  assign switch_ati_val = tx_send;
  assign switch_ati_data = tx_data;
  assign switch_ati_be = tx_last_be;
  
  assign switch_ati_sof = tx_sof;
  assign switch_ati_eof = tx_eof;
  
  assign switch_ati_discrs = tx_discrc;
  assign switch_ati_dispad = tx_dispad;
  assign switch_ati_chksum_ctrl = tx_chksum;
  
  reg [31:0] tx_status;
  reg switch_ati_txstatus_val_d1;
  
  assign switch_ati_ack = switch_ati_txstatus_val_d1;
  
  always @ (posedge clk)
    switch_ati_txstatus_val_d1 <= switch_ati_txstatus_val;

  always @ (posedge clk)
    if (switch_ati_txstatus_val)
      tx_status = {14'd0, switch_ati_txstatus};
  
  assign switch_ati_ena_timestamp = 1'b0; // NOTE: 'switch_ati_timestamp' is unused
  assign switch_ati_pbl = 9'd0; // NOTE: 'switch_ati_tx_watermark' is unused
  
  //
  // Receive state machine
  //
  
  reg rx_active;
  reg rx_capture;
  reg rx_done;
  reg [9:0] rx_last;
  reg [1:0] rx_last_be;
  reg [9:0] rx_word;
  reg rx_flush;
  
  always @ (posedge clk or posedge rst)
    if (rst)
    begin
      rx_active <= 1'b0;
      rx_capture <= 1'b0;
      rx_done <= 1'b0;
      rx_last <= 10'd0;
      rx_last_be <= 2'b00;
      rx_word <= 10'd2;
      rx_flush <= 1'b0;
    end
    else if (lw_h2f_write && (lw_h2f_address == 13'h1000) && (lw_h2f_byteenable == 4'b1111))
    begin
      rx_active <= lw_h2f_writedata[0];
      rx_capture <= 1'b0;
      rx_done <= 1'b0;
      rx_word <= 10'd2;
      rx_flush <= lw_h2f_writedata[31];
    end
    else if (!rx_capture)
    begin
      rx_capture <= switch_ari_val && switch_ari_sof && !rx_done && rx_active;
    end
    else if (switch_ari_val)
    begin
      rx_active <= !switch_ari_eof;
      rx_capture <= !switch_ari_eof;
      rx_done <= switch_ari_eof;
      rx_last <= rx_word - 10'd2;
      rx_last_be <= switch_ari_be;
      rx_word <= rx_word + 10'd1;
    end
  
  //
  // Receive buffer
  //
  
  reg [31:0] rx_buf[1023:0];
  
  always @ (posedge clk)
    if (rx_capture && switch_ari_val)
      rx_buf[rx_word] <= switch_ari_data;
  
  //
  // Receive
  //
   
  reg [31:0] rx_status;
  reg switch_ari_rxstatus_val_d1;
  
  assign switch_ari_ack = switch_ari_rxstatus_val_d1 || rx_capture;
  
  always @ (posedge clk)
    switch_ari_rxstatus_val_d1 <= switch_ari_rxstatus_val;
    
  always @ (posedge clk)
    if (switch_ari_rxstatus_val)
      rx_status = switch_ari_data;
  
  assign switch_ari_frameflush = rx_flush;
  
  assign switch_ari_pbl = 9'd0; // NOTE: 'switch_ari_rx_watermark' is unused
  // NOTE: 'switch_ari_timestamp_val' is unused
  
  //
  // Read from CSRs or buffers
  //
  
  reg read_data_valid;
  reg [31:0] read_data;
  
  always @ (posedge clk or posedge rst)
    if (rst)
      read_data_valid <= 1'b0;
    else
      read_data_valid <= lw_h2f_read;
  
  always @ (posedge clk)
    if (lw_h2f_address == 13'h0000)
      read_data <= {tx_discrc, tx_dispad, tx_chksum, 14'd0, tx_last_be[1:0], tx_last[9:0], tx_done, tx_active};
    else if (lw_h2f_address == 13'h00004)
      read_data <= tx_status;
    else if (lw_h2f_address == 13'h1000)
      read_data <= {18'd0, rx_last_be[1:0], rx_last[9:0], rx_done, rx_active};
    else if (lw_h2f_address == 13'h1004)
      read_data <= rx_status;
    else if (lw_h2f_address[12] == 1'b1)
      read_data <= rx_buf[lw_h2f_address[11:2]];
    else
      read_data <= 32'hFFFFFBAD;

  assign lw_h2f_waitrequest = 1'b0;
  assign lw_h2f_readdatavalid = read_data_valid;
  assign lw_h2f_readdata = read_data;
  
endmodule
