#pragma once

// Unique identifier for all entities
class Entity
{
    unsigned int m_id;
    static unsigned int id_count;   // defaults to 0 (invalid), need to init 1

public:

    Entity()
    {
        // ensure that each entity gets a unique ID
        m_id = id_count++; // assign and increment
    }

    /*
    Entity(Entity& e)
    {
        m_id = e.m_id;
    }
    */

    ~Entity()
    {
    }

    operator unsigned int() { return m_id; } // enables automatic casting to int

    unsigned int id() { return m_id; }
};