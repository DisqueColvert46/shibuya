# Implémentation de Lucas-Kanade

Dans ce projet, on va implémenter la méthode de Lucas-Kanade pour l'estimation du flux optique. Le code s'adapte bien sûr pour diverses utilisations, mais on se contentera ici de le tester sur un flux vidéo en direct du croisement de Shibuya, au Japon (la vidéo est récupérée depuis Youtube).


# Objectif

Le résultat attendu pour une vidéo donnée (une succession d'images suffisamment "proches" pour rendre compte d'un mouvement, voir les conditions dans la partie théorique) est de détecter et rendre compte des mouvements des objets présents dans la vidéo (par des vecteurs rouges dans le cas du code fourni)

# Organisation

Le code est divisé en 3 parties :

 - calcul.c : Contient les fonctions de manipulations matricielles, et l'implémentation de la méthode de résolution par moindres carrées
 - graph.c : Contient la fonction de récupération du flux vidéo via url, ainsi que les fonctions d'interface code/images (Récupération de données de l'image vers le code, mais aussi utilisation de données du code pour dessiner sur l'image)
 - main.c : Contient la fonction principale 'main', qui articule les fonctions des deux fichiers précédents dans une boucle principale pour répondre à l'objectif attendu

Le code est entièrement commenté et ne sera donc pas expliqué en détails dans les documents Markdown suivants :

- maths.md : Description des théories mathématiques impliquées dans le code
- lucas_kanade.md : Description théorique de la méthode de Lucas-Kanade
- remarques.md : Quelques commentaires sur des corrections, optimisations, ou adaptations du code selon l'utilisation voulue

# Mise en place
Le code peut être compilé et lancé tel quel (si le lien Youtube utilisé existe encore).
Pour ma part, la commande suivante dans MinGW64 sur Windows s'exécute proprement :

    gcc main.c calcul.c graph.c -o programme.exe -lavformat -lavcodec -lavutil -lswscale -lSDL2

Il suffit ensuite d'exécuter programme.exe .

