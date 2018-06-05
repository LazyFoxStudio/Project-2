# Marc Garcia - Team Leader
**[Back to main page](https://lazyfoxstudio.github.io/Project-2/)**

## Gameplay Core:
Programming and implementation of all the gameplay mechanics:

### Mage and Paladin Skills and Skill system:
Responsable to create the Hero class and add the command and squad systems to it. Also responsable of the Skill system which includes: Damage the enemies in a specific area, Blit Skill paterns, Skill Logic and Usability in terms of gameplay

### Mage Skills:
The Mage was created for a distance attack. Example, the mage W and E:

<p align="center"> 
<img src="https://i.imgur.com/4VKr2pX.gif" width="600"> 
<img src="https://i.imgur.com/YGPdvb4.gif" width="600">
</p>

### Paladin Skills:
The Paladin was created for a melee attack. Example, the paladin's Q and W:

<p align="center"> 
<img src="https://i.imgur.com/dN8GzNJ.gif" width="600">
<img src="https://i.imgur.com/hIio5hi.gif" width="600">
</p>

### Wave System:
The wave system has the property to generate enemies randomly in the 4 spawners.

* Enemy Spawn Management: Complex management of what to spawn and in which wave.
* Waves difficulty: Management of the difficulty of the game. With each wave the game turns more difficult because there are more enemies and different types.

### AddEntities() and Squad Selection functions:
Creation of the Spawn functions that can be used to spawn allies in the different town buildings and spawn enemies in the 4 differents spawn points. Also the AddHero() function that make possible the creation of the Hero with the skill depending on the selection (Mage or Paladin).

The selection of Squads makes it easier to control units. When the player selects an ally entity all the squad is selected and ready to receive a command.


## Sprite Ordering:
Ordering of the sprites that appear in the screen in function of the Y position or priority in case of death or flying entity.


## QuadTree:
Programming and implementation of the Quadtree to optimize collisions and make the game more fluid. It is visible on the debug mode:

<p align="center">
<img src="https://i.imgur.com/yWvifvL.gif" width="600">
 </p>

## Tutorial Design:
Design of the tutorial and accessibility of the game for people that are new in the RTS genre with [Adria](https://github.com/Adria-F). 

## QA:
Deep knowledge of the rest of the code parts in order to solve bugs and crashes and also to implement new features. Some examples:
Building system, Workers system, UI, Animation, Pathfinding and Gameplay.  

## Art:
* Creating the Title of the Game and the end image with a Warcraft Style:

<p align="center"> 
<a href="https://gyazo.com/ef40c82988f815a9f9d5d688d30a55c3"><img src="https://i.gyazo.com/ef40c82988f815a9f9d5d688d30a55c3.png" alt="https://gyazo.com/ef40c82988f815a9f9d5d688d30a55c3" width="398"/></a>
<a href="https://gyazo.com/10f7022c9cf2de6a00b32cd4a27ed0cb"><img src="https://i.gyazo.com/10f7022c9cf2de6a00b32cd4a27ed0cb.png" alt="https://gyazo.com/10f7022c9cf2de6a00b32cd4a27ed0cb" width="398"/></a>
</p>


* UI art implementation: Buttons images, tutorial arrows, skills sprites...

## Minor Things:
Some other things like use the UI methods to create the units menu, camera movement with arrows...

**[Back to main page](https://lazyfoxstudio.github.io/Project-2/)**
