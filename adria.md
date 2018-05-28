# Adrià Ferrer - UI Designer
[Back to main page](https://lazyfoxstudio.github.io/Project-2/)


## GUI design
In charge of designing all the UI layout of the game, from the menus to all the elements seen in-game.

You can check the [UI Document](https://github.com/LazyFoxStudio/Project-2/wiki/3.-UI-Document) to see all the designed menus, the interaction
between them and the decisions taken.

<a href="https://gyazo.com/2ac6fdfbc6de105cd54cbac7cbfea6c0"><img src="https://i.gyazo.com/2ac6fdfbc6de105cd54cbac7cbfea6c0.jpg" alt="https://gyazo.com/2ac6fdfbc6de105cd54cbac7cbfea6c0" width="1343.2"/></a>
Main menu example.

<a href="https://gyazo.com/6f2710aad06503b1b0a773d2669ea465"><img src="https://i.gyazo.com/6f2710aad06503b1b0a773d2669ea465.jpg" alt="https://gyazo.com/6f2710aad06503b1b0a773d2669ea465" width="1342.4"/></a>
In-game menu example.

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

<a href="https://gyazo.com/ae68247728172ca5e9d9e7fc3c0698cf"><img src="https://i.gyazo.com/ae68247728172ca5e9d9e7fc3c0698cf.gif" alt="https://gyazo.com/ae68247728172ca5e9d9e7fc3c0698cf" width="1000"/></a>

The implementation in code has been done by adapting his research on a [Cutscene Manager](https://adria-f.github.io/Cutscene-Manager/) in order to work
as a sequence of steps or "missions" that have to be accomplished to move forward into the tutorial.

## QA
As part of the team, has contributed on the QA by looking for bugs and solving them when possible
and ensuring that everything works as desired and the correct release build contruction.

[Back to main page](https://lazyfoxstudio.github.io/Project-2/)