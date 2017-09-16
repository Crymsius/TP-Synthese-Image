
//! \file tuto2.cpp utilisation de mesh pour decrire les sommets d'un ou plusieurs triangles et les dessiner.

#include <stdio.h>
#include <cmath>
#include "image.h"
#include "image_io.h"
#include "window.h"
#include "mesh.h"
#include "mat.h"
#include "vec.h"

int imagew = 512;
int imageh = 512;

/* par defaut, openGL dessine les objets qui se trouvent entre -1 et 1 sur x, y et z.
 */
bool isInsidePixel (Point a, Point b, Point c) {
    float minx = std::min(std::min(a.x, b.x), c.x);
    float maxx = std::max(std::max(a.x, b.x), c.x);
    float miny = std::min(std::min(a.y, b.y), c.y);
    float maxy = std::max(std::max(a.y, b.y), c.y);
    return ((maxx - minx) < 1  && (maxy - miny ) < 1);
//   return (((abs((int)a.x - (int)b.x) <= 1) && (abs((int)a.y - (int)b.y) <= 1))||
//            ((abs((int)b.x - (int)c.x) <= 1) && (abs((int)b.y - (int)c.y) <= 1))||
//            ((abs((int)c.x - (int)a.x) <= 1) && (abs((int)c.y - (int)a.y) <= 1)));
}

bool isOutsideImage(Point a, Point b, Point c) {
    return ((a.x < 0 && b.x < 0 && c.x < 0) || (a.y < 0 && b.y < 0 && c.y < 0) || (a.x > imagew && b.x > imagew && c.x > imagew) || (a.y > imageh && b.y > imageh && c.y > imageh));
}

Image& draw (Point a, Point b, Point c, Image& image) {
    if (isOutsideImage(a,b,c)) {
        return image;
    } else if (isInsidePixel(a, b, c)) {
        image((int)a.x, (int)a.y) = Color(1,1,1,1);
        return image;
    } else {
        Point a2((b.x+c.x)/2, (b.y+c.y)/2, 0);
        Point b2((a.x+c.x)/2, (a.y+c.y)/2, 0);
        Point c2((a.x+b.x)/2, (a.y+b.y)/2, 0);
        image = draw (a,c2,b2,image);
        image = draw (b,a2,c2,image);
        image = draw (c,b2,a2,image);
        image = draw (a2,b2,c2,image);
    }
    return image;
}

int main()
{
    Point Pa(-0.6, -0.5, 0);
    Point Pb( 0.5,  0.5, 0);
    Point Pc(-0.6,  0.5, 0);
    
    Image image(imagew, imageh);    // cree une image de 512x512 pixels
    
    // parcourir tous les pixels de l'image
    Transform Vp = Viewport(imagew, imageh);
    Point Pa2 = Vp(Pa);
    Point Pb2 = Vp(Pb);
    Point Pc2 = Vp(Pc);
    
    image = draw(Pa2, Pb2, Pc2, image);
    
//    image(10, 10) = Color(1,1,1,1);
    // enregistre le resultat
    write_image(image, "tp1out.png");
    printf("image enregistrée\n");
    return 0;
}


