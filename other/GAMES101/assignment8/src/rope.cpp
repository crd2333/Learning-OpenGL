#include <iostream>
#include <vector>
#include <memory>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes) {
    // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.

    // create masses
    for (int i = 0; i < num_nodes; i++) {
        Vector2D position = start + (end - start) * (i / (num_nodes - 1));
        // masses.emplace_back(std::make_shared<Mass>(new Mass(position, node_mass, false)));
        masses.emplace_back(new Mass(position, node_mass, false));
    }
    // create springs
    for (int i = 0; i < num_nodes - 1; i++) {
        // springs.emplace_back(std::make_shared<Spring>(new Spring(masses[i], masses[i + 1], k)));
        springs.emplace_back(new Spring(masses[i], masses[i + 1], k));
    }
    // Comment-in this part when you implement the constructor
    for (auto &i : pinned_nodes) {
        masses[i]->pinned = true;
    }
}

void Rope::simulateEuler(float delta_t, Vector2D gravity) {
    double damp_factor = 0.00005;
    for (auto &s : springs) {
        // TODO (Part 2): Use Hooke's law to calculate the force on a node
        auto distance = fabs((s->m1->position - s->m2->position).norm());
        Vector2D unit_vec_b2a = (s->m2->position - s->m1->position) / distance;
        double relative_length = distance - s->rest_length;
        Vector2D relative_velocity = s->m2->velocity - s->m1->velocity;
        s->m1->forces += -s->k * relative_length * unit_vec_b2a;
        s->m2->forces += -s->k * relative_length * (-unit_vec_b2a);
        // add global damping
        s->m2->forces += -damp_factor * dot(unit_vec_b2a, relative_velocity) * unit_vec_b2a;
        s->m1->forces += -damp_factor * dot(unit_vec_b2a, relative_velocity) * (-unit_vec_b2a);
    }

    for (auto &m : masses) {
        if (!m->pinned) {
            // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
            m->forces += gravity * m->mass;
            Vector2D acc = m->forces / m->mass;
            auto last_velocity = m->velocity;
            m->velocity = m->velocity + delta_t * acc;
            // m->position = m->position + delta_t * last_velocity; // for explicit Euler integration
            m->position = m->position + delta_t * m->velocity;   // for semi-implicit Euler integration

            // TODO (Part 2): Add global damping (add at Hooke' Law part)
        }

        // Reset all forces on each mass
        m->forces = Vector2D(0, 0);
    }
}

void Rope::simulateVerlet(float delta_t, Vector2D gravity) {
    double damp_factor = 0.00005;
    for (auto &s : springs) {
        // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet (solving constraints)
        auto distance = fabs((s->m1->position - s->m2->position).norm());
        Vector2D unit_vec_b2a = (s->m2->position - s->m1->position) / distance;
        double relative_length = distance - s->rest_length;
        s->m1->forces += -s->k * relative_length * unit_vec_b2a;
        s->m2->forces += -s->k * relative_length * (-unit_vec_b2a);
    }

    for (auto &m : masses) {
        if (!m->pinned) {
            Vector2D temp_position = m->position;
            // TODO (Part 3.1): Set the new position of the rope mass
            Vector2D acc = m->forces / m->mass + gravity;
            // m->position = m->position + (m->position - m->last_position) + acc * delta_t * delta_t;
            // m->last_position = temp_position;
            // TODO (Part 4): Add global Verlet damping (and comment the last two lines)
            m->position = temp_position + (1 - damp_factor) * (temp_position - m->last_position) + acc * delta_t * delta_t;
            m->last_position = temp_position;
        }
    }
}
}
