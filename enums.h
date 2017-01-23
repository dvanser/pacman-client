#ifndef PACMAN_PACKETS_H
#define PACMAN_PACKETS_H
enum Packet {
    JOIN,
    ACK,
    START,
    END,
    MAP,
    PLAYERS,
    SCORE,
    MOVE,
    MESSAGE,
    QUIT,
    JOINED,
    PLAYER_DISCONNECTED
};
enum Map_objects{
    None,
    Dot,
    Wall,
    PowerPellet,
    Invincibility,
    Score
};
enum Player_state{
    Normal,
    Dead,
    Power,
    Invincibil
};

enum Client_movement{
    Up,
    Down,
    Right,
    Left
};

enum Player_type{
    Pacman,
    Ghost,
};
#endif