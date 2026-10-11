# Lucas-Kanade

Dans ce document, on va établir la méthode de Lucas-Kanade, à partir des hypothèses jusqu'au système linéaire à résoudre.
Le bagage mathématiques nécessaire est précisé dans maths.md.

## Objectif

La méthode de Lucas-Kanade cherche à estimer le flux optique d'une succession d'image, c'est-à-dire à déterminer les mouvements d'intensité lumineuse, et donc le mouvement apparent des objets qui composent les images

## Hypothèse

L'hypothèse fondamentale est la constance du flot au voisinage d'un pixel, cela signifie que pour un pixel fixé, d'une image à l'autre et dans le voisinage de ce pixel les intensités lumineuses peuvent être considérées comme régulières.
Cela signifie que le flux optique (un mouvement d'intensité lumineuse) ne doit pas être plus rapide que la succession d'image puissent en rendre compte d'une image à l'autre, dans le voisinage d'un même pixel

## Equation de flux optique

Soit $I$ l'intensité d'un pixel $(x,y)$ au temps $t$
Par hypothèse, les variations de I sont suffisament petites pour qu'un développement de Taylor ne soit pas abérant :

$$I(x+\delta x, y + \delta y, t + \delta t) = I(x,y,t) + \frac{\partial I}{\partial x}\delta x+\frac{\partial I}{\partial y}\delta y+\frac{\partial I}{\partial t}\delta t$$

Par hypothèse l'intensité est quasi-constante, d'où :

$$0=\frac{\partial I}{\partial x}\delta x+\frac{\partial I}{\partial y}\delta y+\frac{\partial I}{\partial t}\delta t$$


On "divise" alors par $\delta t$ (il ne s'agit pas réellement d'une valeur infinitésimale, mais simplement d'une valeur très faible et bien définie) :

$$0 = \frac{\partial I}{\partial x}V_x+\frac{\partial I}{\partial y}V_y+\frac{\partial I}{\partial t}$$

$V_x$ et $V_y$ sont alors les quantités recherchées, puisqu'il s'agit de la vitesse de déplacement de l'intensité du pixel considéré, il s'agit de l'**équation du flot optique**.
L'objectif étant de mesurer le mouvement apparent des objets, on va chercher une fenêtre de pixels (de taille 1x1, 2x2, 3x3 ou autre). Si on a un mouvement apparent, alors les pixels de la fenêtre vont partager les mêmes $V_x$ et $V_y$. On dispose alors de $N$ pixels caractérisés par $(x_i,y_i,t_i)$ pour $i\in \{1,...,N\}$ qui vérifie la même équation de flot optique. On a alors le système suivant :
$$\begin{cases}
\frac{\partial I}{\partial x_1}V_x+\frac{\partial I}{\partial y_1}V_y = -\frac{\partial I}{\partial t_1} \\
\frac{\partial I}{\partial x_2}V_x+\frac{\partial I}{\partial y_2}V_y = -\frac{\partial I}{\partial t_2} \\
... \\
\frac{\partial I}{\partial x_N}V_x+\frac{\partial I}{\partial y_N}V_y = -\frac{\partial I}{\partial t_N}
\end{cases}$$

Qui se traduit matriciellement par $AV=B$ avec :
$$A = \begin{pmatrix} \frac{\partial I}{\partial x_1} & \frac{\partial I}{\partial y_1} \\ 
. & . \\
. & . \\
\frac{\partial I}{\partial x_N} & \frac{\partial I}{\partial y_N}\end{pmatrix}$$
$$B = \begin{pmatrix} \frac{\partial I}{\partial t_1} \\ 
. \\
. \\
\frac{\partial I}{\partial t_N}
\end{pmatrix}$$
$$V = \begin{pmatrix} V_x \\ V_y \end{pmatrix}$$

Il suffit ensuite de faire confiance à la méthode des moindres carrés pour nous donner une solution exploitable de $V$ pour ce système. À défaut d'une solution exacte, si la méthode des moindres carrés aboutie à un résultat $V$, il s'agira bien de la tendance de déplacement : Si sur 9 pixels, on en trouve 7 qui semble se déplacer vers le haut, et 2 vers le bas, le système n'aura pas de solution exacte mais la méthode des moindres carrés va bien nous donner un vecteur $V$ dirigé vers le haut.
