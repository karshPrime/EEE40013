-- VHDL Test Bench Created from source file multiplier.vhd -- 15:36:36 09/01/2003

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library std;
use std.textio.all;

entity testbench is
end testbench;

architecture behavior of testbench is 

-- ports of UUT
signal Clock    :  std_logic := '0';
signal A        :  unsigned(15 downto 0) := (others => '0');
signal B        :  unsigned(15 downto 0) := (others => '0');
signal Q        :  unsigned(31 downto 0) := (others => '0');
signal Start    :  std_logic := '0';
signal Complete :  std_logic := '0';
signal reset    :  std_logic := '0';
  
  
--  File to log results to
file logFile : TEXT;

constant clockHigh   : time := 50 ns; 
constant clockLow    : time := 50 ns; 
constant clockPeriod : time := clockHigh + clockLow; 
    
signal   simComplete : boolean := false;

begin

   --****************************************************
   -- Clock Generator
   --
   ClockGen:
   process

   begin
      while not simComplete loop
         clock <= '0';  wait for clockHigh;
         clock <= '1';  wait for clockLow;
      end loop;

      wait; -- stop process looping
   end process ClockGen;

   --****************************************************
   -- Stimulus Generator
   --
   Stimulus:
   process

   --*******************************************************
   -- Write a message to the logfile & transcript
   --
   --  message => string to write
   --
   procedure writeMsg ( message : string
                      ) is

   variable assertMsgBuffer : String(1 to 4096); -- string for assert message
   variable writeMsgBuffer : line; -- buffer for write messages

   begin
      write(writeMsgBuffer, message);
      assertMsgBuffer(writeMsgBuffer.all'range) := writeMsgBuffer.all;
      writeline(logFile, writeMsgBuffer);
      deallocate(writeMsgBuffer);
      report assertMsgBuffer severity note;
   end;

   procedure doMultiply( stimulusA : unsigned(15 downto 0);
                         stimulusB : unsigned(15 downto 0)
                         ) is

   variable assertMsgBuffer : String(1 to 4096); -- string for assert message
   variable writeMsgBuffer  : line; -- buffer for write messages
   variable expectedQ       : unsigned(31 downto 0);
   variable actualQ         : unsigned(31 downto 0);

   begin

      write( writeMsgBuffer, string'("A = "), left );
      write( writeMsgBuffer, to_integer(stimulusA) );
      write( writeMsgBuffer, string'(", B = "), left );
      write( writeMsgBuffer, to_integer(stimulusB) );

      --  Fill out with your test sequence for the multiplier

      assertMsgBuffer(writeMsgBuffer.all'range) := writeMsgBuffer.all;
      writeline(logFile, writeMsgBuffer);
      deallocate(writeMsgBuffer);
      report assertMsgBuffer severity note;
   end;

   variable openStatus : file_open_status;

   begin -- Stimulus

      file_open(openStatus, logFile, "results.txt", WRITE_MODE);

      writeMsg( string'("Simulation starting.") );

      -- initial reset
      A     <= (others => '0');
      B     <= (others => '0');
      Start <= '0';

      reset <= '1';
      wait for 10 ns;
      reset <= '0';
      wait until falling_edge(Clock);
      

      -- Test cases - modify as needed.
      doMultiply( "0000000000000010", "0000000000000010" );

      wait for 20 ns;

      writeMsg( string'("Simulation completed.") );

      file_close(logFile);

      simComplete <= true; -- stop clock & simulation

      wait;

   end process Stimulus;

   uut: entity work.Multiplier1Cycle
   port map (
       reset    => reset,
       Clock    => Clock,
       A        => A,
       B        => B,
       Q        => Q,
       Start    => Start,
       Complete => Complete
       );

end;
