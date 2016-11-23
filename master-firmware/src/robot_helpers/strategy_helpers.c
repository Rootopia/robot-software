#include <error/error.h>
#include "position_manager/position_manager.h"
#include "trajectory_manager/trajectory_manager_utils.h"
#include "blocking_detection_manager/blocking_detection_manager.h"
#include "obstacle_avoidance/obstacle_avoidance.h"
#include "trajectory_helpers.h"
#include "beacon_helpers.h"

#include "strategy_helpers.h"

void strategy_map_setup(int32_t robot_size)
{
    /* Define table borders */
    polygon_set_boundingbox(robot_size/2, robot_size/2,
                            3000 - robot_size/2, 2000 - robot_size/2);

    /* Add obstacles */
    poly_t *crater = oa_new_poly(4);
    oa_poly_set_point(crater, 400, 300, 3);
    oa_poly_set_point(crater, 400, 800, 2);
    oa_poly_set_point(crater, 900, 800, 1);
    oa_poly_set_point(crater, 900, 300, 0);

    poly_t *fence = oa_new_poly(4);
    oa_poly_set_point(fence,   0, 250, 3);
    oa_poly_set_point(fence,   0, 480, 2);
    oa_poly_set_point(fence, 850, 480, 1);
    oa_poly_set_point(fence, 850, 250, 0);
}

void strategy_set_opponent_obstacle(int32_t x, int32_t y, int32_t opponent_size, int32_t robot_size)
{
    /* Set opponent obstacle */
    static poly_t *opponent;
    static bool is_initialized = false;

    if (!is_initialized) {
        opponent = oa_new_poly(4);
        is_initialized = true;
    }
    // poly_t *opponent = oa_new_poly(4);

    beacon_set_opponent_obstacle(opponent, x, y, opponent_size, robot_size);

    NOTICE("Opponent obstacle seen at %d %d", x, y);
    NOTICE("Point 0 %d %d", x - (opponent_size + robot_size) / 2, y - (opponent_size + robot_size) / 2);
    NOTICE("Point 1 %d %d", x - (opponent_size + robot_size) / 2, y + (opponent_size + robot_size) / 2);
    NOTICE("Point 2 %d %d", x + (opponent_size + robot_size) / 2, y + (opponent_size + robot_size) / 2);
    NOTICE("Point 3 %d %d", x + (opponent_size + robot_size) / 2, y - (opponent_size + robot_size) / 2);
}

void strategy_auto_position(
        int32_t x, int32_t y, int32_t heading, int32_t robot_size,
        enum strat_color_t robot_color, struct _robot* robot, messagebus_t* bus)
{
    /* Configure robot to be slower and less sensitive to collisions */
    trajectory_set_mode_aligning(&robot->mode, &robot->traj, &robot->distance_bd, &robot->angle_bd);

    /* Go backwards until we hit the wall and reset position */
    trajectory_align_with_wall(robot, bus);
    position_set(&robot->pos, MIRROR_X(robot_color, robot_size/2), 0, 0);

    /* On se mets a la bonne position en x. */
    trajectory_d_rel(&robot->traj, (double)(x - robot_size/2));
    trajectory_wait_for_end(robot, bus, TRAJ_END_GOAL_REACHED);

    /* On se tourne face a la paroi en Y. */
    trajectory_only_a_abs(&robot->traj, 90);
    trajectory_wait_for_end(robot, bus, TRAJ_END_GOAL_REACHED);

    /* On recule jusqu'a avoir touche le bord. */
    trajectory_align_with_wall(robot, bus);

    /* On reregle la position. */
    robot->pos.pos_d.y = robot_size / 2;
    robot->pos.pos_s16.y = robot_size / 2;

    /* On se met en place a la position demandee. */
    trajectory_set_speed(&robot->traj, speed_mm2imp(&robot->traj, 300), speed_rd2imp(&robot->traj, 2.5));

    trajectory_d_rel(&robot->traj, (double)(y - robot_size/2));
    trajectory_wait_for_end(robot, bus, TRAJ_END_GOAL_REACHED);

    /* Pour finir on s'occuppe de l'angle. */
    trajectory_a_abs(&robot->traj, (double)heading);
    trajectory_wait_for_end(robot, bus, TRAJ_END_GOAL_REACHED);

    /* Restore robot to game mode: faster and more sensitive to collision */
    trajectory_set_mode_game(&robot->mode, &robot->traj, &robot->distance_bd, &robot->angle_bd);
}
