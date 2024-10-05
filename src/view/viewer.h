#pragma once

#include "injector/scene.h"

class MyViewer : public SimpleScene {
public:
  void draw(vtkObject* caller, unsigned long eventId, void* callData) override;
};

void MyViewer::draw(vtkObject* caller, unsigned long eventId, void* callData) {
}
