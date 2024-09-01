These are the parts needed to build a 3D cellular automata! 
- [x] generate a random 128 bit set representing the rule
- [x] check whether central cell lives or dies in next step based on values of 2-bit <x,y,z> neighborhood vector plus central cell bit
- [x] print whether each index results in cell living or dying in next step
- [ ] ticker to move time forward
- [ ] data structure to hold state of all cells in world
- [ ] function to generate world state (3D cell grid). Takes in size of grid (x_max,y_max,z_max) and randomizes the value (0 or 1) in each cell of the grid
- [ ] function to updates cells (world state) at each step based on rule map. Takes in args: current_world_state, rule_map and outputs next_world_state