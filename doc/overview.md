# Approaches

## Timed Games
(based on Bouyer et al. [1])

1. Construct TA(s) for Golog program and platform model
1. Construct ATA B_phi with L(B_phi) = L(phi)
1. Construct synchronous product T_A/phi of A and B_phi
1. Construct DT~ (how exactly? p. 12 in [1])
1. Determine strategy from DT~

Advantages:
* Directly describes the full synthesis problem

Disadvantages:
* Complex procedure
* High complexity


## Deterministic Timed Automata

(based on Ničković and Piterman [2])

1. Construct TA(s) for the MTL specification (following [2])
1. Construct TA(s) for Golog program and platform model
1. Apply reachability analysis on the the product automaton
1. Synthesize a controller from a path that reaches a final state

Questions:
* (How) can we synthesize the strategy? Is reachability enough?

Advantages:
* Use existing tools for reachability analysis
* Lower complexity (double exponential)

Disadvantages:
* No implementation available
* It's unclear how to go from a TA representation of an MTL formula to synthesis

---

[1] P. Bouyer, L. Bozzelli, and F. Chevalier, “Controller Synthesis for MTL Specifications,” in Proceedings of the 17th International Conference on Concurrency Theory (CONCUR), 2006, pp. 450–464.

[2] D. Ničković and N. Piterman, “From MTL to deterministic timed automata,” in Formal Modeling and Analysis of Timed Systems, 2010, pp. 152–167.
