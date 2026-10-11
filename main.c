#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "calcul.h"
#include "graph.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

#define SDL_MAIN_HANDLED // Pour éviter une erreur de WinMain à la compilation
#include <SDL2/SDL.h>
// La documentation https:// ffmpeg.org/doxygen/trunk/index.html pour les AV est excellente

int main(int argc, char *argv[])
{
	int targetWidth = 426; // Les dimensions souhaitées pour une image
	int targetHeight = 240;
	int chunk = 3; // Lors d'un calcul sur un pixel, on considère la boule fermé de rayon chunk autour du pixel comme un même bloc
	
	char stream_url[16384];
	int stream_url_taille = sizeof(stream_url);
	char *source_url = "https://www.youtube.com/watch?v=dfVK7ld38Ys";
	if (get_m3u8_url(source_url, stream_url, stream_url_taille) != 0) {
		return 1;
	}
	printf("On a l'url !\n");
	
	// On construit la fenêtre de rendu
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "Erreur SDL %s\n", SDL_GetError());
		return 1;
	}
	SDL_Window *fenetre = SDL_CreateWindow("Mon rendu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, targetWidth, targetHeight, 0);
	SDL_Renderer *moteur_rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED);
	SDL_Texture *texture = SDL_CreateTexture(moteur_rendu, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, targetWidth, targetHeight);
	// La matrice de rendu en rgb uint32_t
	uint32_t *matrice_affiche = (uint32_t *) malloc(targetWidth * targetHeight * sizeof(uint32_t));
	
	av_log_set_level(AV_LOG_ERROR);
	avformat_network_init(); // On récupére un flux de l'extérieur, donc on initialise les librairies réseau FFMPEG.
	// Il faudra les désinitialiser à la fin avec avformat_network_deinit()
	AVFormatContext *format_context = NULL;
	
	if (avformat_open_input(&format_context, stream_url, NULL, NULL) < 0) { // On oubliera pas de refermer l'entrée avformat_close_input
		fprintf(stderr, "avformat_open_input renvoie un code erreur\n");
		return 1;
	}
	
	avformat_find_stream_info(format_context, NULL); // On récupére les informations du flux
	int video_stream_index = -1; // Initialisation de l'index de la piste vidéo dans le flux
	AVCodecParameters *codec_params = NULL; // Initialisation des paramètres de décodage
	const AVCodec *codec = NULL; // Initialisation du décodeur
	// On parcourt tout les flux du contexte (piste video, audio etc...)
	// On utilise '->' car format_contexte est un POINTEUR vers la structure AVFormatContext
	// On utilise un unsigned int car on aura pas de flux négatif, donc autant utiliser deux fois plus d'entiers positifs si jamais il y a trop de flux
	
	for (unsigned int i = 0; i < format_context->nb_streams; i++) { 
		if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			// On a la suite de pointeurs suivant : AVFormatContext -> AVStream -> AVCodecParameters
			// codec_type est un enum AVMediaType, codec_id est un enum AVCodecID
			video_stream_index = i;
			codec_params = format_context->streams[i]->codecpar;
			codec = avcodec_find_decoder(codec_params->codec_id);
			break;
		}
	}
	
	if (video_stream_index == -1 || codec == NULL) { // On vérifie si la boucle a bien donné des valeurs
		fprintf(stderr, "La boucle n'a rien trouvé !\n");
	}
	
	// On alloue la mémoire et on construit le décodeur
	AVCodecContext *codec_context = avcodec_alloc_context3(codec); // On oubliera pas le avcodec_free_context()
	avcodec_parameters_to_context(codec_context, codec_params); // On complète le contexte avec les paramètres
	avcodec_open2(codec_context, codec, NULL); // On construit le contexte avec le codec
	
	uint8_t *matrice_grise = (uint8_t *)malloc(targetWidth*targetHeight*sizeof(uint8_t)); // On alloue la mémoire pour la matrice de gris (pas de couleur dans le modèle de Lucas-Kanade)
	uint8_t *matrice_grise_ancienne = (uint8_t *)malloc(targetWidth*targetHeight*sizeof(uint8_t)); // De même avec une matrice d'une frame de retard pour calculer la "dérivée partielle en temps" de l'intensité
	
	// On oubliera pas de libérer la mémoire à la fin avec free()
	if (matrice_grise == NULL || matrice_grise_ancienne == NULL) { // On vérifie que le pointeur est non nul
		fprintf(stderr, "Erreur d'allocation\n");
		return 1;
	}
	
	// Création du convertisseur en gris et redimensionné
	struct SwsContext *sws_context = sws_getContext(codec_context->width, codec_context->height, codec_context->pix_fmt, targetWidth, targetHeight, AV_PIX_FMT_GRAY8, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	
	// Allocation pour les packets et frames (à libérer à la fin)
	AVPacket *packet = av_packet_alloc();
	AVFrame *frame = av_frame_alloc();
	int nb_frame = 0; // On met un compteur de frame, pour garder une trace du "temps" écoulé
	
	// Création des pointeurs au format adapté pour sws_scale(), on a qu'un seul Plan (d'où les 3 autres NULL et 0)
	uint8_t *target_data[4] = { matrice_grise, NULL, NULL, NULL };
	int target_linesize[4] = { targetWidth, 0, 0, 0 };
	printf("Traitement en temps reel\n");
	
	// Variable d'expérience utilisateur
	bool on_continue = true; // Condition de boucle classique
	SDL_Event event; // Interaction avec l'utilisateur
	int frequence_img = 33; // On va créer un délai pour limiter les à-coup, cette constante est la fréquence souhaitée
	int statistiques_flux[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // Première idée naïve pour calculer les flux -> voir documentation
	int nb_vecteur_traite = 1; // On compte les vecteurs pris en compte dans statistiques_flux pour faire la moyenne
	while (av_read_frame(format_context, packet) >= 0 && on_continue) {
		if (packet->stream_index == video_stream_index) { // On vérifie qu'il s'agit bien de la piste vidéo trouvée préalablement
			if (avcodec_send_packet(codec_context, packet) == 0) {
				while (avcodec_receive_frame(codec_context, frame) == 0) { // C'est là qu'on récupére une frame
					Uint32 debut_tick = SDL_GetTicks();
					
					// On actualise les deux matrices grises en échangeant les pointeurs mémoires
					uint8_t *temp = matrice_grise_ancienne;
					matrice_grise_ancienne = matrice_grise;
					matrice_grise = temp; // matrice_grise devient pointe vers l'ancienne mais on la remodifie juste après -> Une-Deux Ronaldo - Roberto Carlos
					
					target_data[0] = matrice_grise; // Sans ça, il va prendre la mauvaise adresse une fois sur deux
					
					sws_scale(sws_context, (const uint8_t * const *) frame->data, frame->linesize, 0, codec_context->height, target_data, target_linesize);
					nb_frame++; // On a récupéré une frame
					
					for (int pixel = 0; pixel < targetWidth * targetHeight; pixel++) {
						uint8_t pixel_gris = matrice_grise[pixel];
						// On va effectuer la traduction Gris -> ARGB, en utilisant le concept de décalage sachant que chaque Alpha, Rouge, Vert et Bleu fait 8 bits
						// '|' représente un OU bit à bit
						matrice_affiche[pixel] = (255 << 3*8) | (pixel_gris << 2*8) | (pixel_gris << 1*8) | (pixel_gris << 0*8);
					}
					
					// Maillage de drapeau, un pour chaque zone de 20*20 pixels
					for (int i = 0; i < targetHeight; i++) {
						for (int j = 0; j < targetWidth; j++) {
							if (i % 3 == 2 && j % 3 == 2) {
								int pixel = i * targetWidth + j;
								float vecteur[2];
								calculFluxPonctuel(targetWidth, targetHeight, chunk, pixel, matrice_grise, matrice_grise_ancienne, vecteur);
								float x = vecteur[0];
								float y = vecteur[1];
								float norme = sqrtf(x*x + y*y);
								if (norme > 0.5f) {
									int type = direction_vecteur(vecteur);
									statistiques_flux[type] = statistiques_flux[type] + 1;
									nb_vecteur_traite += 1;
									dessiner(targetWidth, targetHeight, vecteur, pixel, matrice_affiche);
								}
							}
						}
					}
					
					if (nb_frame % 10 == 0) {
						for (int i = 0; i < 8; i++) {
							int prop = (100*statistiques_flux[i]) / nb_vecteur_traite;
							printf("Les vecteurs de type %d sont en proportions %d\n",i, prop);
							statistiques_flux[i] = 0;
						}
						nb_vecteur_traite = 1; // Pas 0 pour éviter une division par zéro, et statistiquement ça ne changera rien
					}
					
					// On affiche dans la fenetre
					SDL_UpdateTexture(texture, NULL, matrice_affiche, targetWidth * sizeof(uint32_t));
					SDL_RenderClear(moteur_rendu);
					SDL_RenderCopy(moteur_rendu, texture, NULL, NULL);
					SDL_RenderPresent(moteur_rendu);
					
					// Action Utilisateur
					while (SDL_PollEvent(&event)) {
						if (event.type == SDL_QUIT) {
							on_continue = 0;
						}
					}
					
					Uint32 fin_tick = SDL_GetTicks();
					Uint32 duree = fin_tick - debut_tick;
					if (duree < frequence_img) {
						SDL_Delay(frequence_img - duree); // On évite les à-coup si le traitement va plus vite que la musique
					}
				}
			}
		av_packet_unref(packet); // On oublie la référence du packet traité
		}
	}
	
	// Libérations promises
	free(matrice_affiche);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(moteur_rendu);
	SDL_DestroyWindow(fenetre);
	SDL_Quit();
	av_frame_free(&frame);
	av_packet_free(&packet);
	free(matrice_grise);
	avcodec_free_context(&codec_context);
	avformat_close_input(&format_context);
	avformat_network_deinit();
	return 0;
}