# Marc Garcia - Team Leader
**[Back to main page](https://lazyfoxstudio.github.io/Project-2/)**

## Gameplay Core:
Programming and implementation of all the gameplay mechanics:

### Mage adn Paladin Skills and Skill system:
Responsable to create the Hero class and add the command and squad systems to it. Also responsable of the Skill system wich includes: Damage the enemies in a specific area, Blit Skill paterns, Skill Logic and Usability in terms of gameplay

### Mage Skills:
The Mage was created for a distance attack:

<img src="https://i.imgur.com/zoVVF82.gif" width="400"> <img src="https://i.imgur.com/4VKr2pX.gif" width="400"> 

<p align="center"> 
<img src="https://i.imgur.com/YGPdvb4.gif" width="400">
</p>

### Paladin Skills:
The Paladin was created for a melee attack:

<img src="https://i.imgur.com/dN8GzNJ.gif" width="400"> <img src="https://i.imgur.com/hIio5hi.gif" width="400"> 

<p align="center"> 
<img src="https://i.imgur.com/1fyhwXd.gif" width="400">
</p>

### Wave System:
The wave system has the property to generate enemies randombly in the 4 spawners.

* Enemy Spawn Management: Complex management of what to spawn and in which wave.
* Waves difficulty: Management of the difficulty of the game. Each wave the game turns more difficult because there are more enemies and differents.

### Squad Selection and AddEntities() functions:
Creation of the Spawn functions that can be used to spawn allies in the different town buildings and spawn enemies in the 4 differents spawn points. Also the AddHero() function that make possible create the Hero with the skill depending on the selection (Mage or Paladin).


## Sprite Ordering:
Ordering of the sprites that appear in the screen in funciton of the Y position or priority in case of death or flying entity.


## QuadTree:
Programming and implementation of the Quadtree to optimize collisions and make the game more fluid. It is visible on the debug mode:
<img src="https://i.imgur.com/yWvifvL.gif" width="400">

## Tutorial Design:
Design of the tutorial and accessivility of the game for people that are new in the RTS genre with [Adria](https://github.com/Adria-F). 

## QA:
Deep knowledge of the rest code parts in order to solve bugs and crash and also to implement new features. Some examples:
Bulding system, Workers system, UI, Animation, Pathfinding and Gameplay.  

## Art:
Creating the Tittle of the Game and the end image with a Warcraft Style:
<a href="https://gyazo.com/ef40c82988f815a9f9d5d688d30a55c3"><img src="https://i.gyazo.com/ef40c82988f815a9f9d5d688d30a55c3.png" alt="https://gyazo.com/ef40c82988f815a9f9d5d688d30a55c3" width="353"/></a>
<a href="https://gyazo.com/10f7022c9cf2de6a00b32cd4a27ed0cb"><img src="https://i.gyazo.com/10f7022c9cf2de6a00b32cd4a27ed0cb.png" alt="https://gyazo.com/10f7022c9cf2de6a00b32cd4a27ed0cb" width="398"/></a>

UI art implementation: Buttons images, tutorial arrows, skills sprites...

## Minor Things:
Some other things like use the UI methods to create the units menu, camera movement accessivility with arrows...

**[Back to main page](https://lazyfoxstudio.github.io/Project-2/)**
