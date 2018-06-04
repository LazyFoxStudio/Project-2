# Rodrigo de Pedro
**[Back to main page](https://lazyfoxstudio.github.io/Project-2/)**       

As the team's coder, my duty is to ensure all code implemented into the project is stable, optimized and readable. To achieve this, I've stablished coding guidelines and rules, as well as personally reviewed most code introduced by any member of the team. I've served also as consultant for any code-related question in the project, while keeping the team's objectives achievable and realistic. Finally, I've also been in charge of fixing most of bugs that appeared during the development.

The core mechanics I've personally implented are:       

## Pathfinding

In-game units use a combination of A*, flow fields and separation as pathfinding algorithm, as well as an offset system for formations. The simultaneous combination of this three systems allow units to move around the map efficiently and logically.

### A* algorithm

This algorithm is fairly known in programming environments. It's very efficient in close distances, but can consume a lot of resources when travelling to a far point or when called by several units simultaneously. Thus, it's only used for combat movement, as distances between enemies are short.

### Flow fields

This algorithm is based on create a surface over the map containing the directions a unit must follow to reach a specific destination. It's main benefit is that a single call allows to create a surface which can be shared among any number of units as long as they have in common the destination, being specially useful in our game where units are created and controlled in groups. This is the system used by units when moving outside the combat.

### Separation

The separation algorithm, as it's name states, is used to avoid units from overlapping over each other. It works like two magnets of the same polarity, repelling other nearby units until they are not colliding anymore. This system is always active to ensure units are always in proper positions.

### Pooled pathfinding

Pathfinding operations are among the most resource expensive mechanics of a strategy game. If an operation takes too long to finish, it will stop the game with a lag spike. To avoid this, pooled pathfinding has been implemented. In pooled pathfinding, a fixed time interval is set at the start of the application. If an operation takes more than the stablished time, it is paused and resumed at the next frame, allowing for a smooth and constant framerate.

## Offsets and formations

To be able to move and hold in formation, units have a set of stablished offsets that are applied along the commander position. These move accordingly as the group moves and rotates. If a unit breaks formation, let's say, because it has found an obstacle along the way, the remaining units of the squad will carry on at a slower pace to allow the other unit to reach them.

## Combat system

As other similar games, Alliance: the last bastion uses unit stats as range, defense or line of sight which define how they will behave in combat. However, simply implementing this stats to in-game mechanics is not enough to give the feeling of "smart" units. One of the examples are melee combatants given the map is based on square tiles. As the number of spaces surrounding an enemy which would allow to melee attack are limited, units must know how to distribute themselves along the tiles to deal the more damage possible.

To overcome this problem, squads of units store which tiles around them are available for melee enemies. Then, when those enemies are found, they distribute those tiles among them so all possible tiles are filled. If there are more enemies than tiles, the remanining enemies will cunningly wait for a spot to be freed to enter the battle.

### MultiTarget attack

The Juggernaut is the most poweful enemy the player can face in the game, so it must suppose a threat great enough to insipire fear. Therefore, we designed a special feature for it, the MultiTarget Attack. It allows this unit to attack three enemies simultaneously, casting a powerful AoE attack into the closer enemy while shooting arrows to two other nearby units.

## Flying units

Some units in the game have the ability to fly, going over obstacles and non-flying units. These units required from dedicated code to allow them to use this characteristic.

## Command system

One of the problems developers face when implementing agents working with a state machine is that it doesn't allow agents to have "memory". These agents will be able to execute a serie of actions based on their current state and very likely change state when finished. However, once this change occurs, the units has no means to know if there are more actions the player wants to be performed, forcing every individual command to be issued once the previous has finished.

To avoid this, we have implented a command system. Instead of using states, each unit and squad has a command list, which will be executed in order. Each command is a self-executable piece of code, composed of Init, Update and Stop methods, which defines an action inside the game as move or attack. This allows to store several commands in a unit list, so we can easily concatenate actions.

## Other

- Entity controller: memory managing for entities, utility methods...
- Parabollic particles: arrows that fly in a curve in the air before reaching it's target
- Collider, range and line of sight systems: allows units to chase enemies and help nearby allies in a realistic way.
- Entity DataBase: a vector which stores information about units allowing to easy access for different operations.

**[Back to main page](https://lazyfoxstudio.github.io/Project-2/)**       
