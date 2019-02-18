# -*- coding: utf-8 -*-
# vim: ts=4 sw=4 et

from game import Board, Player

MAX_OFFSET = 1

class Situation:
    """Description of visible state to a player.
        to_front -- list of distances from the player
                    to the front (abs), begining from the
                    y-1 to y+1, where y is the current y-position
                    of the player.
                    If the player is close to the boundary 
                    (y == 0 or y == ROWS-1)
                    then the value in the y-1 or y+1 is None.
        ours -- list of players on the front of the same role
                as the player in poistions y-1, y, y+1
        enemies -- list of players on the front of the opposite role
                in positions y-1, y, y+1
        example:

                to_front = [10, 9, 10]
                ours = [0, 1, 0]
                enemies = [0, 0, 2]

    """
    def __init__(self, to_front, ours, enemies, radar):
        self.to_front = to_front or [0] * (MAX_OFFSET*2+1)
        self.ours = ours or [0] * (MAX_OFFSET*2+1)
        self.enemies = enemies or [0] * (MAX_OFFSET*2+1)
        self.radar = radar


    def set_front_at_offset(self, offset,  to_front, ours, enemies):
        """ sets the values for front situation at the given offset
            (-1, 0, -1)

            example:
            sit.set_front_at_offset(-1, 10, 0, 1)
        """
        self.to_front[MAX_OFFSET  + offset] = to_front
        self.ours[MAX_OFFSET + offset] = ours
        self.enemies[MAX_OFFSET + offset] = enemies

    
        
class Front(Board):
    def __init__(self, width, height, players_num):
        """Initializes game board  with width and height,
        adds players_num playes on both sides
        """
        Board.__init__(self)
        self.width = width
        self.height = height
        self.front = [width/2 for _ in range(height)]
        self.positions = {}

        # place  front in positions
        for y, x in enumerate(self.front):
            self.positions[(x, y)] = 'f'

        # place players
        for n in range(players_num):
            player = Soldier(0, n, 'left')
            self.add_player(player, 'left')

            player = Soldier(width-1, height-1-n, 'right')
            self.add_player(player, 'right')

    def add_player(self, player, role):
        Board.add_player(self, player, role)

        self.positions[(player.x, player.y)] = player

    def update_ui(self):
        for y in range(self.height):
            row = ""
            for x in range(self.width):
                if (x,y) in self.positions:
                    what = self.positions[(x, y)]
                    if what == 'f':
                        row += '#'
                    else:
                        # A if C else B
                        # C?A:B
                        row += 'L' if what.role == 'left' else 'R'
                else:
                    row += ' '
            print(row)


    def front_can_move(self, x, y):
        """Check whether front can be moved at position x,y"""
        if y > 0:
            if abs(self.front[y-1] - x) > 1:
                return False
        if y < self.height-1:
            if abs(self.front[y+1] - x) > 1:
                return False
        return True


    def collect_soldiers(self, y, side):
        soldiers = []
        pos = self.front[y] + side
        while self.positions.get((pos, y)):
            soldiers.append(self.positions[(pos, y)])
            pos += side
        return soldiers


    def radar_sym(self, x, y):
        """radar sym returns:
            # -- for boundary
            f -- for front
            l -- for left player
            r -- for right playr
            space -- for empty space
        """
        if 0 <= x < self.width and 0 <= y < self.height:
            if (x, y) in self.positions:
                if self.positions[(x,y)] != 'f':
                    return 'l' if self.positions[(x,y)].role == 'left' else 'r'
                return 'f'
            return ' '
        return '#'

    def get_radar(self, p):
        x, y = p.x, p.y
        radar = [
            [self.radar_sym(px, py) for px in range(x-1, x+2)] for py in range(y-1, y+2)
        ]
        return radar

    def get_player_situation(self, p):
        x, y = p.x, p.y
        rng = range(p.y-1, p.y+2)
        to_front = [abs(p.x - self.front[y]) if 0 <= y < self.height else None for y in rng]
        enemies, ours = ([len(self.collect_soldiers(y, -1))  if 0 <= y < self.height else None for y in rng],
                        [len(self.collect_soldiers(y, 1)) if 0 <= y < self.height else None for y in rng])

        if p.role == 'left':
            enemies, ours = ours, enemies
        
        radar = self.get_radar(p)

        sit = Situation(to_front, ours, enemies, radar)
        return sit

    def update(self):
        for y, x in enumerate(self.front):
            left_soldiers = self.collect_soldiers(y, -1)
            right_soldiers = self.collect_soldiers(y, 1)
            bal = len(left_soldiers) - len(right_soldiers)
            if bal > 0:
                # move front right
            
                if self.front_can_move(x+1, y) and x+len(right_soldiers) < self.width - 1:
                    # move right soldiers
                    for r in reversed(right_soldiers):
                        self.move(r, 1, 0)
                    del self.positions[(x,y)]
                    self.positions[(x+1, y)] = 'f'
                    self.front[y] += 1
                    for l in left_soldiers:
                        self.move(l, 1, 0)

            elif bal < 0:
                # move front left
                if self.front_can_move(x-1, y) and x-len(left_soldiers) > 0:
                    # move right soldiers
                    for l in reversed(left_soldiers):
                        self.move(l, -1, 0)
                    del self.positions[(x,y)]
                    self.positions[(x-1, y)] = 'f'
                    self.front[y] -= 1
                    for r in right_soldiers:
                        self.move(r, -1, 0)

                        
        

    def move(self, player, dirx, diry):
        """Check direction and make real move of the player.
            returns false in case if the movement is impossible
            """
        if dirx*dirx + diry*diry > 1:
            return False
        x, y = player.x + dirx, player.y + diry
        if 0 <= x < self.width and 0 <= y < self.height:
            if (x, y) not in self.positions:
                del self.positions[(player.x, player.y)]
                self.positions[(x, y)] = player
                player.x, player.y = x, y
                return True
        return False
    

class Soldier(Player):
    def __init__(self, x, y, role):
        self.x = x
        self.y = y
        self.health = 10
        self.role = role


    def select_dir(self, sit):
        inf = float('inf')
        weight = lambda n: inf if sit.to_front[n] is None else \
                 sit.to_front[n] + (sit.ours[n]  - sit.enemies[n])

        sorted_pos = sorted(range(3), key=weight)

        self.dirx = 1 if self.role == 'left' else -1
        self.diry = 1 - sorted_pos[0]
        if self.diry:
            if sit.radar[1+self.diry][1] != ' ':
                self.dirx = -self.dirx
        if sit.radar[1][1+self.dirx] != ' ':
            self.dirx = 0

    def step(self):
        pass

    def try_step(self):
        sit = self.board.get_player_situation(self)
        #
        #
        #
        self.select_dir(sit)
        self.board.move(self, self.dirx, self.diry)
       

game  = Front(50, 25, 12)
game.update_ui()
while 1:
    game.try_step()
    game.update()
    game.update_ui()
    x = raw_input()
 
