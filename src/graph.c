#include "graph.h"
#include <stdlib.h>

graph_point_t* new_point(uint8_t x, uint8_t y, graph_point_t* next) {
  graph_point_t* new = (graph_point_t*)malloc(sizeof(graph_point_t));
  // need some sort of error catch

  new->x = x;
  new->y = y;
  new->next = next;

  return new;
}

graph_point_t* prepend_point(uint8_t x, uint8_t y, graph_point_t* head) {
  graph_point_t* new = new_point(x, y, head);
  head = new;
  return head;
}

graph_point_t* remove_back(graph_point_t* head) {
  graph_point_t* cursor = head;
  graph_point_t* new_tail = 0;
  while (cursor->next != 0) {
    new_tail = cursor;
    cursor = cursor->next;
  }
  new_tail->next = 0;
  free(cursor);

  return new_tail;
}

graph_t create_graph(uint8_t size, uint8_t startx, uint8_t starty,
                     uint8_t width, uint8_t height) {
  graph_display_options_t disp_opts = {.startx = startx,
                                       .starty = starty,
                                       .width = width,
                                       .height = height,
                                       .draw_point = 1,
                                       .draw_line = 0,
                                       .circle_radius = 2,
                                       .forecolour = C_BLUE,
                                       .backcolour = C_BLACK};

  graph_t graph = {
      .head = 0, .tail = 0, .size = size, .count = 0, .disp = disp_opts};

  clear_graph(&graph);

  return graph;
}

void clear_graph(graph_t* graph) {
  UG_FillFrame(graph->disp.startx, graph->disp.starty,
               graph->disp.startx + graph->disp.width,
               graph->disp.starty + graph->disp.height, 
               graph->disp.backcolour);
}

void draw_graph(graph_t* graph) {
  graph_display_options_t* opts = &graph->disp;

  clear_graph(graph);
  graph_point_t* cursor = graph->head;

  uint8_t count = 0;
  uint16_t y;
  uint16_t x_offset = (opts->width - opts->circle_radius) / (graph->size - 1);
  uint16_t y_start = opts->starty + opts->height;

  while (cursor != 0) {
    if ((y = y_start - cursor->y) > opts->starty) {
      UG_FillCircle(count * x_offset + opts->circle_radius, y,
                    opts->circle_radius, opts->forecolour);
      count++;
    }

    cursor = cursor->next;
  }
}

void add_point(uint8_t x, uint8_t y, graph_t* graph) {
  graph->head = prepend_point(x, y, graph->head);
  graph->count++;
  if (graph->count >= graph->size) {
    remove_back(graph->head);
  }
}