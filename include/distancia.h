#ifndef DISTANCIA_H
#define DISTANCIA_H

typedef enum { OBSTACLE_IS_VERY_CLOSE, OBSTACLE_IS_CLOSE, OBSTACLE_IN_MEDIUM_DISTANCE, OBSTACLE_IS_FAR, OBSTACLE_NOT_PRESENT } ObstacleDistance;

ObstacleDistance to_dist_range(const int distance);

#endif
