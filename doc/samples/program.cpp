/*
** program.cpp
** Login : <ctaf@speedcore>
** Started on  Sun Oct 16 00:42:09 2011
** $Id$
**
** Author(s):
**  -  <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <iostream>
#include "program.chh"

void start() {
};

void resume(void *data, size_t sz) {
};

int main(int argc, char *argv[])
{
  qi::Application app("foobar");

  //app.connect("resume", resume);
  //app.connect("start", start);
  //could have been a method of a class
  app.onResume(&resume);
  app.onStart(&start);

  //block run the main loop
  //will either call start of resume
  return app.run();
}

int main_crgdiag(int argc, char *argv[])
{
  qi::Application app("foobar");
  qi::Xar         xar("mytest.xar");
  qi::Diagram     dg = xar.diagram("testfoo");
  qi::Animation   an = xar.animation("testbar");

  //may not be necessary, by default
  app.onResume(dg, &Diagram::resume);
  app.onStart(dg, &Diagram::start);

  //block run the main loop
  //will either call start of resume
  return app.run();
}



//WRITING A C++ BEHAVIOR

class Behavior {
public:
  onStart();
  onStop();
  onResume();
  onSuspend();
};

//box that multiply two input
class MyFooBox : qi::Box {
  void start();
  void stop();
  void resume();
  void suspend();

  void slota();
  void slotb();

  void init() {
    bang("start", start);
    bang("stop", stop);
    slot("a", slota);
    slot("b", slotb);
    signal<int>("result");
    properties<bool>("invertSign", invertSign, setInverSign);
  };
  //input and properties?
};

//actions can be linked => connect(a1, "finished", a2, "start")
class Action {
public:
  Action();

slots:
  void start();
  void stop();
  void suspend();
  void resume();

signals:
  void finished();
  void error();
};

//a graph of connected actions
class ActionGraph : Action {

};

class Diagram {
};


//synchronous action?
int main(int argc, char *argv[])
{
  qi::Application  app;
  qi::Action      *act = new qi::Animation();
  qi::Service     *svc = new qi::Service();

  app.play(act);
  return app.run();
}


//class RemoteAction ?
