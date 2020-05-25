# Pseudo Code (Timed Games)

1. Get $DT_\sim$ (Todo: add details here :) ).
1. Perform breadth-first search on the unfolding of $DT_\sim$ (unfold on demand) and label states as follows:
    * label fixed points as successful
    * label bad states as unsuccessful
    * label states with no successors as dead
    * otherwise: compute successors in the tree
1. Afterwards: label bottom-up: successful and dead nodes are marked with TRUE, unsuccessful nodes with FALSE. For internal nodes: if all successors are labelled with TRUE, label with TRUE, otherwise with FALSE (Todo: revise this, I am not 100% sure aboute the ALL successors).
1. If the root is labeled with TRUE, there exists a winning strategy.
