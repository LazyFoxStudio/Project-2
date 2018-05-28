# Adrià Ferrer - UI Designer

## GUI design
In charge of designing all the UI layout of the game, from the menus to all the elements seen in-game.

You can check the [UI Document](https://github.com/LazyFoxStudio/Project-2/wiki/3.-UI-Document) to see all the designed menus, the interaction
between them and the decisions taken.

The art of the menus has been updated from the one in the UI Document to the actual one by [Jaume Surís](https://lazyfoxstudio.github.io/Project-2/jaume),
following the guidelines defined in the UI Document and under the supervision of Adrià Ferrer.

The minimap has been implemented by [Hugo Bo](https://lazyfoxstudio.github.io/Project-2/hugo), as part of his [research](https://hugo-bo-diaz.github.io/Minimap-research/).

## GUI code implementation
Responsible of the implementation of the UI system:

	- The modules for all the basic elements (buttons, text, images, windows...).
	
	- The more complex elements, compund by the basic ones.
	
	- The menu system containing the necessary elements and easy toggling on and off.
	
	- Different interaction with the menus elements (swap scene, assign hotkeys, produce troops, perform abilities...).
	
	- All the graphic feedback and game information needed for the player (except particles).
	
	- Connection with all the other modules in order to interact with the game (create troops, create buildings, manage workers on buildings, next wave enemies, squads management and resource management).
	
	- Create a system to read all the UI composition from an XML document and to be as easy to understand as possible, so it can be easily changed without touching the code.
	
## Tutorial
Designed the tutorial, how would it look and the diferent steps needed to understand the basics of the game.

The implementation in code has been done by adapting his research on a [Cutscene Manager](https://adria-f.github.io/Cutscene-Manager/) in order to work
as a sequence of steps or "missions" that have to be accomplished to move forward into the tutorial.

## QA
As part of the team, has contributed on the QA by looking for bugs and solving them when possible
and ensuring that everything works as desired and the correct release build contruction.