#include "graph.h"
#include <stdlib.h>

static uint16_t _get_x_val(uint8_t count, uint16_t x_offset,
                           graph_display_options_t* opts);

graph_point_t* new_point(uint16_t x, uint16_t y, graph_point_t* next) {
  graph_point_t* new = (graph_point_t*)malloc(sizeof(graph_point_t));
  // need some sort of error catch

  new->x = x;
  new->y = y;
  new->next = next;

  return new;
}

graph_point_t* prepend_point(uint16_t x, uint16_t y, graph_point_t* head) {
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

graph_t create_graph(uint8_t size, uint16_t startx, uint16_t starty,
                     uint16_t width, uint16_t height) {
  graph_display_options_t disp_opts = {.startx = startx,
                                       .starty = starty,
                                       .width = width,
                                       .height = height,
                                       .draw_point = 1,
                                       .draw_line = 0,
                                       .circle_radius = 1,
                                       .forecolour = C_BLUE,
                                       .backcolour = C_BLACK,
                                       .outline_colour = C_FLORAL_WHITE,
                                       .margin = 5};

  graph_t graph = {
      .head = 0, .tail = 0, .size = size, .count = 0, .disp = disp_opts};

  clear_graph(&graph);

  return graph;
}

void clear_graph(graph_t* graph) {
  UG_FillFrame(graph->disp.startx + graph->disp.margin,
               graph->disp.starty + graph->disp.margin,
               graph->disp.startx + graph->disp.width - graph->disp.margin,
               graph->disp.starty + graph->disp.height - graph->disp.margin,
               graph->disp.backcolour);
}

void draw_graph_frame(graph_t* graph) {
  UG_DrawFrame(graph->disp.startx, graph->disp.starty,
               graph->disp.startx + graph->disp.width,
               graph->disp.starty + graph->disp.height,
               graph->disp.outline_colour);
}

void draw_graph(graph_t* graph) {
  graph_display_options_t* opts = &graph->disp;

  clear_graph(graph);
  draw_graph_frame(graph);

  graph_point_t* cursor = graph->head;

  uint8_t count = 0;
  uint16_t x1, y1;
  // times by 100 to avoid floating point math
  uint16_t x_offset = (opts->width * (uint16_t)100) / graph->size;
  uint16_t height_margin = opts->height - opts->margin;
  uint16_t y_start = opts->starty + height_margin;

  while (cursor != 0) {
    if (cursor->y <= height_margin) {
      // draw left to right
      x1 = _get_x_val(count, x_offset, opts);
      y1 = y_start - cursor->y;

      if (opts->draw_point)
        UG_DrawCircle(x1, y1, opts->circle_radius, opts->forecolour);

      if (opts->draw_line) {
        if (cursor->next != 0) {
          UG_DrawLine(x1, y1, _get_x_val(count + 1, x_offset, opts),
                      (cursor->next->y <= height_margin)
                          ? y_start - cursor->next->y
                          : opts->starty + opts->margin,
                      opts->forecolour);
        }
      }
    }

    count++;
    cursor = cursor->next;
  }
}

void add_point(uint16_t x, uint16_t y, graph_t* graph) {
  graph->head = prepend_point(x, y, graph->head);
  graph->count++;
  if (graph->count >= graph->size) {
    remove_back(graph->head);
  }
}

uint16_t _get_x_val(uint8_t count, uint16_t x_offset,
                    graph_display_options_t* opts) {
  return opts->width - opts->margin - (count * x_offset) / 100;
}