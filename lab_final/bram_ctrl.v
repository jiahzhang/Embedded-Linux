`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/30/2021 01:30:04 PM
// Design Name: 
// Module Name: bram_ctrl
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module bram_ctrl(
        input wire clk, 
        input wire reset, 
        input wire start_accel,
        output reg start, 
        input wire out_ready, 
        input wire [15:0] num_points,
        output reg [31:0] x_out,
        output reg [31:0] y_out,
        output reg [31:0] z_out,
        input wire [31:0] x_in,
        input wire [31:0] y_in,
        input wire [31:0] z_in,
        output reg [31:0] addr_bram, 
        output reg [31:0] dout_bram, 
        input wire [31:0] din_bram, 
        output wire clk_bram,
        output reg en_bram,
        output wire rst_bram,
        output reg [3:0] we_bram,
        output reg finish
    );
        
        reg [3:0] state;
        reg [3:0] next_state;
        reg [15:0] curr_points;
        reg [31:0] next_addr;
        
        localparam  
            IDLE               = 0,
            START_READ         = 1,
            READ_X             = 2,
            READ_Y             = 3,
            READ_Z             = 4,
            ROTATE             = 5,
            WAIT               = 6,
            WRITE_X            = 7,
            WRITE_Y            = 8,
            WRITE_Z            = 9,
            HOLD               = 10;
            
        
        assign clk_bram = clk;
        assign rst_bram = ~reset;
        
        //outputs
        always@(negedge clk) begin
            if(~reset) begin
                en_bram <= 0;
                we_bram <= 0;
                dout_bram <= 0;
                addr_bram <= 0;
                next_state <= IDLE;
                x_out <= 0;
                y_out <= 0;
                z_out <= 0;
                curr_points <= 0;
                next_addr <= 0;     
                finish <= 0;      
            end
            else begin
                case(state)
                    IDLE:
                        begin
                            en_bram <= 0;
                            we_bram <= 0;
                            addr_bram <= 0;
                            next_addr <= 0;
                            curr_points <= 0;
                            finish <= 0; 
                            if(start_accel) next_state <= START_READ;
                            else next_state <= IDLE;
                        end
                    START_READ:
                        begin
                            en_bram <= 1;
                            we_bram <= 0;
                            addr_bram <= next_addr;
                            curr_points <= curr_points + 1;
                            if(curr_points == num_points) next_state <= HOLD;
                            else next_state <= READ_X;
                        end
                    READ_X:
                        begin
                            en_bram <= 1;
                            we_bram <= 0;
                            addr_bram <= addr_bram + 4;
                            x_out <= din_bram;
                            next_state <= READ_Y;
                        end
                    READ_Y:
                        begin
                            en_bram <= 1;
                            we_bram <= 0;
                            addr_bram <= addr_bram + 4;
                            y_out <= din_bram;
                            next_state <= READ_Z;
                        end                        
                    READ_Z:
                        begin
                            en_bram <= 1;
                            we_bram <= 0;
                            next_addr <= addr_bram + 4;
                            z_out <= din_bram;
                            next_state <= ROTATE;
                        end
                    ROTATE:
                        begin
                            en_bram <= 0;
                            we_bram <= 0;
                            start <= 1;
                            next_state <= WAIT;
                        end
                    WAIT:
                        begin
                            start <= 0;
                            if(out_ready) next_state <= WRITE_X;
                            else next_state <= WAIT;
                        end
                    WRITE_X:
                        begin
                            en_bram <= 1;
                            we_bram <= 4'b1111;
                            dout_bram <= x_in;
                            addr_bram <= addr_bram - 8;
                            next_state <= WRITE_Y;
                        end
                    WRITE_Y:
                        begin
                            en_bram <= 1;
                            we_bram <= 4'b1111;
                            dout_bram <= y_in;
                            addr_bram <= addr_bram + 4;
                            next_state <= WRITE_Z;
                        end                       
                    WRITE_Z:
                        begin
                            en_bram <= 1;
                            we_bram <= 4'b1111;
                            dout_bram <= z_in;
                            addr_bram <= addr_bram + 4;
                            next_state <= START_READ;
                        end
                    HOLD:
                        begin
                            en_bram <= 0;
                            we_bram <= 0;
                            finish <= 1;
                            if(start_accel) next_state <= HOLD;
                            else next_state <= IDLE;
                        end
                    endcase
                end
            end
        
        //state transitions
        always@(posedge clk) begin
            if(!reset) begin
               state <= IDLE;
            end
            else state <= next_state;
        end
        
endmodule
