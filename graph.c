#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "graph.h"
#include "calcul.h"

int get_m3u8_url(char *source_url, char *out_url, int out_url_taille) { // Récupération du lien du flux vidéo
	char cmd[1024];
	int verif = snprintf(cmd, sizeof(cmd), "yt-dlp --quiet --no-warnings --get-url --format 229 \"%s\"", source_url); // On construit la commande yt-dlp, 232 est la piste vidéo de qualité maximale
	printf("Execution de la commande\n");
	FILE* canal = popen(cmd,"r"); // On execute la commande dans le fichier canal
	if (canal == NULL) {
		printf("canal est vide\n");
		return -1;
	}
	if (fgets(out_url, out_url_taille, canal) != NULL) {
		out_url[strcspn(out_url,"\r\n")] = '\0'; // On remplace le saut de ligne "\r\n" par un code de fin
	} else {
		fprintf(stderr, "fgets renvoie le pointeur NULL\n");
		pclose(canal);
		return -1;
	}
	pclose(canal);
	return 0;
}


void calculVoisinage(int width, int height, int chunk, int pixel, int *sortie) { // sortie est de taille (2*chunk+1)²
	// On va calculer les élements du voisinage selon la dimension chunk, en faisant attention aux bords
	// P.S : On va probablement mettre de côté les pixels trop proches du bord, mais au cas où la fonction sera capable de les gérer
	int pixel_i = pixel / width;
	int pixel_j = pixel % width;
	for (int i = -chunk; i <= chunk; i++) {
		for (int j = -chunk; j <= chunk; j++) {
			int ni = pixel_i + i;
			int nj = pixel_j + j;
			if (nj < 0 || nj >= width || ni < 0 || ni >= height) {
				sortie[(i+chunk)*(2*chunk+1)+(j+chunk)] = -1; // On met la valeur -1 si on sort des limites, c'est un cas à gérer dans les fonctions qui font appel à celle-ci
			} else {
				sortie[(i+chunk)*(2*chunk+1)+(j+chunk)] = ni*width+nj; // (2*chunk+1) est la taille d'une ligne de sortie
			}
		}
	}
}

void calculIntensitePonctuel(int width, int height, int pixel, uint8_t *matrice, uint8_t *matrice_ancienne, float *sortie) {
	// sortie est un tableau de 3 flotants, la dérivée partielle de l'intensité sur les abscisses Ix, celle sur les ordonnées Iy, et la temporelle
	int vois[9]; 
	/*On calcule les points autour (on ne se servira pas des diagonales). Pour les dérivées, on prend la convention abscisses et ordonnées croissantes
		0, 1, 2,			Autrement dit un flux positif en abscisse se dirige vers la droite de l'image,
		3, 4, 5,			Un flux positif en ordonnée se dirige vers le BAS de l'image
		6, 7, 8,			où 4 représente le pixel étudié*/
	calculVoisinage(width, height, 1, pixel, vois); // On va supposer qu'il n'y a pas de -1 dans vois (=sortie), autrement dit cette fonction est appelé sur des pixels qui ne sont pas aux bords
	float Ix = (float) ((int)matrice[vois[5]] - (int)matrice[vois[3]]) / 2.0f;
	float Iy = (float) ((int)matrice[vois[7]] - (int)matrice[vois[1]]) / 2.0f;
	float It = (float) ((int)matrice[vois[4]] - (int)matrice_ancienne[vois[4]]); // On peut remplacer vois[4] par pixel, c'est simplement pour le parallèle avec les lignes précédentes
	sortie[0] = Ix;
	sortie[1] = Iy;
	sortie[2] = It;
}

void calculFluxPonctuel(int width, int height, int chunk, int pixel, uint8_t *matrice, uint8_t *matrice_ancienne, float *sortie) {
	// sortie est un vecteur de deux flotants
	int N = (2*chunk+1)*(2*chunk+1); // Nombre de points considérés
	int vois[N];
	calculVoisinage(width, height, chunk, pixel, vois);
	float matriceA[2 * N]; // La matrice de N lignes, 2 colonnes, dont la première colonne contient les Ix de chaque point, et la seconde les Iy
	float matriceB[N]; // La matrice de N lignes, 1 colonnes (c'est un vecteur) contenant les It
	for (int i = 0; i < N; i++) {
		// x -> i*2 + 0 et y i*2 + 1
		float intensite[3];
		calculIntensitePonctuel(width, height, vois[i], matrice, matrice_ancienne, intensite);
		matriceA[i*2 + 0] = intensite[0];
		matriceA[i*2 + 1] = intensite[1];
		matriceB[i] = 0 - intensite[2];
	}
	moindresCarres(N, 2, matriceA, N, 1, matriceB, sortie);
}

void dessiner(int width, int height, float *vecteur, int pixel, uint32_t *matrice_affiche) {
	// Pour l'instant je ne traite pas les problèmes de bord d'écran, je testerais la fonction en plein milieu
	float x = vecteur[0];
	float y = vecteur[1];
	float norme = sqrtf(x*x + y*y);
	if (norme > 0.5f) {
		x *= (1 / norme);
		y *= (1 / norme);
		for (int t = 0; t < 10; t++) { // On crée un paramètre de temps pour calculer la direction du pixel selon le vecteur
			int i = (int) (pixel / width) + y*t;
			int j = (int) (pixel % width) + x*t;
			if (i >= 0 && j >= 0 && i < width && i < height) {
				matrice_affiche[i*width+j] = (255 << 3*8) | (255 << 2*8); // On colore en rouge le pixel
			}
		}
	}
}

int direction_vecteur(float *vecteur) {
	/* Renvoie la tendance de direction du vecteur : (on récupère l'argument principal par atan2(y,x) pour un vecteur (x,y)
	1, 2, 3, <---- Sur la fenêtre de rendu, un vecteur a pour type de direction l'entier qu'il pointe sur ce cadran
	0, ., 4,
	7, 6, 5
	*/
	float arg = atan2(vecteur[1], vecteur[0]);
	int tor = (int) (arg * 4.0f / M_PI);
	return tor+4;
}