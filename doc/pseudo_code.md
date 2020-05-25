# Pseudo Code (Timed Games)

1. Construct $\mathit{DTA}(\Sigma, \delta)$ for Golog program $\delta$ with BAT $\Sigma$ for some platform model $\mathcal{R}$
1. Construct an ATA $B_\varphi$ with $\mathcal{L}^*(\mathcal{B}_\varphi) = \mathcal{L}^*(\varphi)$, following [^OW2005]

   TODO: More details on the construction of $B_\varphi$

1. ~~Construct $\mathcal{T}_{\mathcal{A}/\varphi}$, the synchronous product of $\mathcal{A}$ and $\mathcal{B}_\varphi$~~

   _Do not construct the complete synchronous product, it has uncountably many states_
1. Construct $DT_\sim$ on the fly as follows:
   * Start with $w_0 = H(s_0)$, where $H(s)$ is a canonical word representing $s$; $H(\cdot)$ is an equivalence relation that partitions $\mathcal{T}_{\mathcal{A}/\varphi}$

     TODO: More details on $H$
   * A transition $\mathcal{C} \overset{a, g, Y}{\longrightarrow} \mathcal{C}'$
     is added on-demand if there are $s_1 \in H^{-1} (w_1), s_2 \in H^{-1} (w_2)$ with $s_1 \overset{a,g,Y}{\longrightarrow} s_2$

     Question: How do we compute $H^{-1}$?

1. Perform breadth-first search on the unfolding of $DT_\sim$ (unfold on demand) and label states as follows:
    * label fixed points as successful
    * label bad states as unsuccessful
    * label states with no successors as dead
    * otherwise: compute successors in the tree
1. Afterwards: label bottom-up: successful and dead nodes are marked with TRUE, unsuccessful nodes with FALSE. For internal nodes: if all successors are labelled with TRUE, label with TRUE, otherwise with FALSE (Todo: revise this, I am not 100% sure aboute the ALL successors).
1. If the root is labeled with TRUE, there exists a winning strategy.

[^OW2005]: J. Ouaknine and J. Worrell, “On the decidability of metric temporal logic,” in 20th Annual IEEE Symposium on Logic in Computer Science (LICS’ 05), 2005, pp. 188–197, doi: 10.1109/LICS.2005.33.
