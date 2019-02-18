# -*- coding: utf-8 -*-
# vim: ts=4 sw=4 et

class Player:
    def try_step(self):
        pass

    def step(self):
        pass


class Board:
    def __init__(self):
        self.players = []
        self.roles = {}

    def add_player(self, player, role):
        self.players.append(player)

        if role not in self.roles:
            self.roles[role] = []
        
        self.roles[role].append(player)
        player.board = self

    def role_order(self, role):
        """override me"""
        return 1

    def _each_role_player(self, func):
        roles = sorted(self.roles.keys(),
            key=lambda x: self.role_order(x))

        for role in roles:
            print("Role %s" % role)
            for p in self.roles[role]:
                func(p)


    def try_step(self):
        print("Try step")
        self._each_role_player(lambda x: x.try_step())
        
    def step(self):
        print("Step")
        self._each_role_player(lambda x: x.step())


    def update_ui(self):
        pass

    def update(self):
        self.try_step()
        self.step()
        self.update_ui()

           
