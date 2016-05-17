/*
   Copyright 2013 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * Simple I/O module for SD card
 *
 * Authors: Mathias Eide Eriksen & Casper Munch Hansen
 *
 */

package io

import Chisel._
import Node._
import patmos.Constants._


import ocp._



object Sdmodule extends DeviceObject {
  var Sd_freq = -1

  def init(params: Map[String, String]) = {
    Sd_freq = getPosIntParam(params, "Sd_freq")
  }

  def create(params: Map[String, String]) : Sdmodule = {
    Module(new Sdmodule(CLOCK_FREQ,Sd_freq))
  }

  trait Pins {
      val sdmodulePins = new Bundle() {
      val SD_CS = Bits(OUTPUT, 1)
      val MOSI = Bits(OUTPUT, 1)
      val SD_CLK = Bits(OUTPUT, 1)
      val MISO = Bits(INPUT, 1)
  }
}
}

class Sdmodule(clk_freq: Int, Sd_freq : Int) extends CoreDevice() {

  override val io = new CoreDeviceIO() with Sdmodule.Pins

  val sd_clkReg        = Reg(init = UInt(width = 1))
  val sd_csReg        = Reg(init = UInt(width = 1))
  val sd_clkdlyReg     = Reg(init = Bits(0,1))
  val sd_MosiReg      = Reg(init = Bits(1,1))
  val sd_MisoReg      = Reg(init = UInt(width = 1))
  val sd_MisolatchReg      = Reg(init = Bits(1,1))

  val rdselectReg = Reg(init = UInt(width = 2))
  val ctrlReg = Reg(init = UInt(width = 2))
  val rdyReg = Reg(init =  UInt(width = 1))
  val cmddataReg = Reg(init =  UInt(width = 48))
  val cmdReg = Reg(init =  UInt(width = 6))

  val clk_enReg = Reg(init = Bits(0,1))
  val resetReg = Reg(init = Bits(0,1))
  val clktickReg      = Reg(init = UInt(1, 32))
  val txcountReg      = Reg(init = Bits(0,14))

  val rxtxcountReg = Reg(init = UInt(width = 11))
  val txdataReg = Reg(init = UInt(width = 32))

  val cmdrespReg = Reg(init =  UInt(width = 40))
  val blockdataReg = Reg(init =  UInt(width = 4096))
  val blockdataQueue = Module(new Queue(Bits(width = 32), 128))


  val countReg = Reg(init = Bits(10,13))
  val clkdivReg = Reg(init =  UInt( (clk_freq/2)/Sd_freq ,width = 32)) // default from xml file
  val doneReg = Reg(init = Bits(0,1))

  val sd_idle :: sd_check_busy :: sd_nops :: sd_write_cmd :: sd_clear_cmd :: sd_wait_resp :: sd_clear_wait :: sd_rx8_resp :: sd_clear_rx8 ::sd_rx40_resp :: sd_write_block :: sd_rx_token :: sd_clear_token :: sd_read_block :: sd_clear_read :: sd_rxtx_crc :: Nil  = Enum(UInt(), 16)
  val sd_state            = Reg(init = sd_idle)
  val rdDataReg = Reg(init = UInt(width = 32))

   val latchinReg = Reg(init = Bits(0,1))
   val shiftinReg = Reg(init = Bits(0,1))
   val shiftoutReg = Reg(init = Bits(0,1))
   val busyReg = Reg(init = Bits(0,1))
   
  
  blockdataQueue.io.enq.bits     := io.ocp.M.Data
  blockdataQueue.io.enq.valid    := Bool(false)
  blockdataQueue.io.deq.ready    := Bool(false)
  
   latchinReg := UInt(0)
    shiftinReg := UInt(0)
    shiftoutReg := UInt(0)
    sd_clkdlyReg := sd_clkReg
  doneReg := UInt(0)
  sd_MosiReg :=  Bits(1) 
  rdDataReg := Mux(io.ocp.M.Addr(3) === Bits(1),
                      Mux(io.ocp.M.Addr(2) === Bits(1),
                          blockdataQueue.io.deq.bits,                    //11
                          Cat(Bits(0, width = 24), cmdrespReg(39,32))),  //10
                      Mux(io.ocp.M.Addr(2) === Bits(1),
                          cmdrespReg(31,0),                              //01
                          Cat(Bits(0, width = 31), rdyReg)))             //00
              
    //FSM controlling command and response

    switch (sd_state) {

        is (sd_idle) {
          rdyReg := busyReg
          sd_csReg  := UInt(1)
          cmdReg := cmddataReg(45,40)
          rxtxcountReg := UInt(0)
          sd_MisolatchReg := UInt(1)
          clktickReg  := UInt(1)
          txcountReg  := UInt(0)
          when(ctrlReg === Bits(0)) {sd_state := sd_check_busy}
          when(ctrlReg === Bits(1)) {sd_state := sd_nops}
          when(ctrlReg === Bits(2)) {sd_state := sd_write_cmd
            sd_csReg  := UInt(0)
            sd_MosiReg := cmddataReg(47)}
        }

        is (sd_check_busy){
          sd_csReg := UInt(0)
          busyReg := sd_MisoReg
          when(rxtxcountReg === clkdivReg){
            sd_state := sd_idle
            } .otherwise { rxtxcountReg := rxtxcountReg + UInt(1)}

        }


        is (sd_nops) {
          rdyReg := Bits(0)
          ctrlReg := Bits(0)
          countReg := cmddataReg(12,0)
          when(doneReg === UInt(1)) {
              clktickReg  := UInt(1)
              txcountReg  := UInt(0)
              sd_state := sd_idle }
          }

        is (sd_write_cmd) {
          rdyReg := Bits(0)
          ctrlReg := Bits(0)
          sd_csReg  := UInt(0)
          countReg := UInt(48)
          sd_MosiReg := cmddataReg(47) 
          cmdrespReg(19,0) :=  UInt(978705)  //EEF11
          cmdrespReg(39,20) :=  UInt(912091) //DEADB
           when(doneReg === UInt(1)) {
              sd_MosiReg := UInt(1)
              sd_state := sd_clear_cmd}
          
        }

        is (sd_clear_cmd) {
          clktickReg  := UInt(1)
          txcountReg  := UInt(0)
          sd_state := sd_wait_resp   
          //when(cmdReg === UInt(17) ){sd_state := sd_clear_token }
        }

        is (sd_wait_resp) {
          rdyReg := Bits(0)
          sd_csReg  := UInt(0)
          countReg := UInt(65)
          cmdrespReg(19,0) :=  UInt(1043946) //FEDEA
          cmdrespReg(39,20) := UInt(1043946) //FEDEA
          when(sd_MisolatchReg === UInt(0)){
            sd_state := sd_clear_wait
          }
          when(doneReg === UInt(1)) { 
              sd_state := sd_idle }
          }

        is (sd_clear_wait) {
          clktickReg  := UInt(1)
          txcountReg  := UInt(0)
          when(cmdReg === UInt(8) || cmdReg === UInt(58)){ 
                      sd_state := sd_rx40_resp} 
          .otherwise{ sd_state := sd_rx8_resp }
        }
            
        
        is (sd_rx8_resp) {
          rdyReg := Bits(0)
          sd_csReg  := UInt(0)
          countReg := UInt(7)
          when(doneReg === UInt(1)) {
            when(cmdReg === UInt(17) && (cmdrespReg(7,0) === Bits(0,8))){
                sd_state := sd_clear_rx8
              } .otherwise {
                sd_state := sd_idle }
          }
        }

        is (sd_clear_rx8) {
          clktickReg  := UInt(1)
          txcountReg  := UInt(0)
          when(cmdReg === UInt(24) || cmdReg === UInt(17)){
          when(cmdReg === UInt(17)){
          sd_state := sd_rx_token     
          }  
          when(cmdReg === UInt(24)){
                  rxtxcountReg := UInt(0)
                  sd_MosiReg := txdataReg(31)
                  txdataReg(15,0) := UInt(65534) //FFFE
                  txdataReg(31,16) := UInt(65535) //FFFF
                  sd_state := sd_write_block
          }
          }.otherwise { sd_state := sd_idle }      
        }

        is (sd_rx40_resp) {
          rdyReg := Bits(0)
          sd_csReg  := UInt(0)
          countReg := UInt(40)
          when(doneReg === UInt(1)) { 
              sd_state := sd_idle }
          }
         

        is (sd_rx_token) {
          rdyReg := Bits(0)
          sd_csReg  := UInt(0)
          countReg := UInt(8)
          when(doneReg === UInt(1)) {
            rxtxcountReg := rxtxcountReg + UInt(1)
            when(cmdrespReg(7,0) =/= Bits(255,8) ){
               when(cmdrespReg(7,0) === Bits(254,8) ){ 
                cmdrespReg(19,0) :=  cmdrespReg(19,0)
                cmdrespReg(27,20) := UInt(10)  //x0A
                cmdrespReg(39,28) := rxtxcountReg 
                sd_state := sd_clear_token
               } .otherwise {
                cmdrespReg(19,0) :=  cmdrespReg(19,0)
                cmdrespReg(39,20) := UInt(782078)  //xBEEFE
                sd_state := sd_idle 
               }
            }
            when(rxtxcountReg === UInt(4096)) { 
              cmdrespReg(19,0) :=  cmdrespReg(19,0)
              cmdrespReg(39,20) := UInt(912094)  //xDEADE
              sd_state := sd_idle } 
            }
          }

        is (sd_clear_token) {
          clktickReg  := UInt(1)
          txcountReg  := UInt(0)
          rxtxcountReg := UInt(0)
          sd_state := sd_read_block         
        }


        is (sd_read_block) {
          rdyReg := Bits(0)
          sd_csReg  := UInt(0)
          countReg := UInt(32)
         when(doneReg === UInt(1)) {
            rxtxcountReg := rxtxcountReg + UInt(1)
            blockdataQueue.io.enq.valid    := Bool(true)
            blockdataQueue.io.enq.bits := txdataReg
          }
          when(rxtxcountReg === UInt(128)){
              sd_state := sd_clear_read }        
        }

        is (sd_clear_read) {
          clktickReg  := UInt(1)
          txcountReg  := UInt(0)
          sd_state := sd_rxtx_crc      
        }

        is (sd_rxtx_crc) {
          rdyReg := Bits(0)
          sd_csReg  := UInt(0)
          countReg := UInt(16)
         when(doneReg === UInt(1)) {
           sd_state := sd_clear_wait

         }        
        }


        is (sd_write_block) {
          rdyReg := Bits(0)
          sd_csReg  := UInt(0)
          countReg := UInt(32)
          sd_MosiReg := txdataReg(31)
          when(doneReg === UInt(1)) { 
            rxtxcountReg := rxtxcountReg + UInt(1)
            blockdataQueue.io.deq.ready    := Bool(true)
            txdataReg := blockdataQueue.io.deq.bits
          }
          when(rxtxcountReg === UInt(128)){
              sd_state := sd_rxtx_crc }
          
        }


    }



    //SD clock generation and latch+shift functionality

    when( sd_state === sd_nops || sd_state === sd_write_cmd || sd_state ===  sd_wait_resp || 
          sd_state ===  sd_rx8_resp || sd_state ===  sd_rx40_resp || sd_state ===  sd_write_block || 
          sd_state ===  sd_rx_token || sd_state ===  sd_read_block || sd_state === sd_rxtx_crc) {
      
      when( txcountReg === Cat(countReg,Bits(0))) {
        sd_clkReg  := UInt(0)
            when(clktickReg === clkdivReg){
              clktickReg  := UInt(1)
              txcountReg  := UInt(0)
              clk_enReg := UInt(0)
              doneReg := UInt(1)
              }
          .otherwise { clktickReg     := clktickReg + UInt(1)}
      } .otherwise {
        when(clktickReg === clkdivReg){
          clktickReg  := UInt(1)
          sd_clkReg  :=  sd_clkReg ^ UInt(1)
          txcountReg     := txcountReg + UInt(1)          
        }
          .otherwise {
            clktickReg     := clktickReg + UInt(1)
          }
      }
      //when(clktickReg === UInt(1)){
       //when(sd_clkReg === UInt(1)){
        when(sd_clkReg === UInt(1) && sd_clkdlyReg === UInt(0)){
            sd_MisolatchReg := sd_MisoReg
            latchinReg := UInt(1) 
        }//}

      //when(clktickReg === UInt(1)){
       when(sd_clkReg === UInt(0) && sd_clkdlyReg === UInt(1)){
            shiftinReg := UInt(1)
            when(sd_state ===  sd_rx8_resp || sd_state ===  sd_rx40_resp || sd_state ===  sd_rx_token){
              cmdrespReg := Cat(cmdrespReg(38,0),sd_MisolatchReg)
            }
            when(sd_state ===  sd_read_block){ //SD_CLK edge
              txdataReg := Cat(txdataReg(30,0),sd_MisolatchReg)
            }
            }
        //  }
      when(clktickReg === clkdivReg - UInt(1)){
        when(sd_clkReg === UInt(1) ){
            
            shiftoutReg := UInt(1)
            when(sd_state === sd_write_cmd){
              cmddataReg := Cat(cmddataReg(46,0),Bits(0))
            }
            
            when(sd_state ===  sd_write_block){
              txdataReg := Cat(txdataReg(30,0),Bits(1))
            }
        
          }
       }
    } 



   //Default response
  val respReg = Reg(init = OcpResp.NULL)
  respReg := OcpResp.NULL

   //Write to Sdmodule
  when(io.ocp.M.Cmd === OcpCmd.WR) {
    respReg := OcpResp.DVA
    when(io.ocp.M.Addr(7,2) === Bits(1, 6)){ 
      ctrlReg := io.ocp.M.Data(1, 0)
    }
    when(io.ocp.M.Addr(7,2) === Bits(2, 6)){
      cmddataReg(31,0) := io.ocp.M.Data
      cmddataReg(47,32) := cmddataReg(47,32)
    }
    when(io.ocp.M.Addr(7,2) === Bits(3, 6)){
      cmddataReg(31,0) := cmddataReg(31,0)
      cmddataReg(47,32) := io.ocp.M.Data(15,0)
    }
    when(io.ocp.M.Addr(7,2) === Bits(8, 6)){
      clkdivReg := io.ocp.M.Data
    }
    blockdataQueue.io.enq.valid    := io.ocp.M.Addr(8) === Bits(1)
  }

  // Read current state of Sdmodule
    when(io.ocp.M.Cmd === OcpCmd.RD) {
      respReg := OcpResp.DVA
        blockdataQueue.io.deq.ready   := io.ocp.M.Addr(8) === Bits(1) 
    }

  // Connections to master
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := rdDataReg


  // Connection to pins
  io.sdmodulePins.SD_CS :=  sd_csReg
  io.sdmodulePins.MOSI :=  sd_MosiReg
  io.sdmodulePins.SD_CLK :=  sd_clkReg
  sd_MisoReg := UInt(1) //Mux(latchinReg === Bits(1), shiftinReg, shiftoutReg) //io.sdmodulePins.MISO//
  

}

