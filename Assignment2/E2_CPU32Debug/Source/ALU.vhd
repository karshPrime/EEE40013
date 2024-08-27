--******************************************************************
--  ALU.vhd
--******************************************************************

library ieee;
use ieee.std_logic_1164.ALL;
use ieee.numeric_std.all;

use work.cpupackage.ALL;
-- synopsys translate_off
use work.debug.ALL;
-- synopsys translate_on

entity alu is
    Port ( aluOp     : in  std_logic_vector(2 downto 0);
           operand1  : in  unsigned(31 downto 0);
           operand2  : in  unsigned(31 downto 0);
           aluOutput : out unsigned(31 downto 0);
           Z,N,V,C   : out std_logic
         );
end alu;

architecture Behavioral of alu is

constant zero       : unsigned( 3 downto 0) := "0000";
signal   aluOutputx : unsigned(32 downto 0);

begin

ALUProcess:
process ( aluOp, operand1, operand2, aluOutputx )
begin

   V <= '0';

   case aluOp is
      when ALUopAdd =>
         aluOutputx <= ('0' & operand1) + ('0' & operand2);
         V  <= (not aluOutputx(31) and (operand1(31) and operand2(31))) or
               (aluOutputx(31) and (not operand1(31) and not operand2(31)));
      when ALUopSub =>
         aluOutputx <= ('0' & operand1) - unsigned('0' & operand2);
         V   <= (not aluOutputx(31) and (operand1(31) and not operand2(31))) or
                (aluOutputx(31) and (not operand1(31) and operand2(31)));
      when ALUopAnd =>
         aluOutputx <= '0' & (operand1 and operand2);
      when ALUopOr =>
         aluOutputx <= '0' & (operand1 or operand2);
      when ALUopEor =>
         aluOutputx <= '0' & (operand1 xor operand2);
      when ALUopRor =>
         -- Rotate through carry
         aluOutputx <= operand1(operand1'right) & '0' & (operand1(operand1'left downto 1));
      when ALUopSwap =>
         aluOutputx <= '0' & operand2(15 downto 0) & operand2(31 downto 16);
      when others =>
         aluOutputx <= (others => 'X');
   end case;
 
   N <= aluOutputx(31);
   C <= aluOutputx(32);
   if (aluOutputx(31 downto 0) = x"00000000") then
      Z <= '1';
   else
      Z <= '0';
   end if;

   aluOutput <= aluOutputx(31 downto 0);

end process ALUProcess;
 
end Behavioral;

 
