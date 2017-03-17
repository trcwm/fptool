--
--
--
--
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use IEEE.MATH_REAL.ALL;

entity tb is
end entity;

architecture behavior of tb is
  signal a : SIGNED(15 downto 0);         -- Q(1,15)
  signal result : SIGNED(21 downto 0);    -- Q(4,18)
  signal result_real : REAL;
begin

  proc_sim: process
    variable result_real : real;
  begin
    a <= "0111111111111111";
    wait for 10 ns;
    result_real := real(TO_INTEGER(result));
    result_real := result_real*2.0**(-18.0);
    report "The value of 'result' is " & real'image(result_real);
    
    wait;
  end process proc_sim;
  