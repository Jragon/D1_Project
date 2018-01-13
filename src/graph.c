#include "graph.h"
#include <stdlib.h>

static uint16_t _get_x_val(uint8_t count, uint16_t x_offset,
                           graph_display_options_t* opts);
static uint16_t _get_y_val(uint16_t y, uint16_t starty, uint16_t height,
                           uint16_t margin);

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

graph_point_t* remove_back(graph_dataset_t* dataset) {
  graph_point_t* cursor = dataset->head;
  graph_point_t* new_tail = 0;
  while (cursor->next != 0) {
    new_tail = cursor;
    cursor = cursor->next;
  }
  new_tail->next = 0;
  dataset->finalval = cursor->y;
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

  graph_t graph = {.size = size, .disp = disp_opts};

  clear_graph(&graph);

  return graph;
}

graph_dataset_t create_dataset(char* name, UG_COLOR colour) {
  return (graph_dataset_t){
      .name = name, .count = 0, .head = 0, .colour = colour, .redraw = 1};
}

void add_dataset(graph_t* graph, graph_dataset_t* dataset) {
  if (graph->dataset_count < MAX_DATA_SET_PER_GRAPH) {
    graph->dataset[graph->dataset_count] = dataset;
    graph->dataset_count++;
  }
}

void clear_graph(graph_t* graph) {
  UG_FillFrame(graph->disp.startx + graph->disp.margin,
               graph->disp.starty + graph->disp.margin,
               graph->disp.startx + graph->disp.width - graph->disp.margin,
               graph->disp.starty + graph->disp.height - graph->disp.margin,
               graph->disp.backcolour);
}

void clear_line(graph_dataset_t* dataset, graph_t* graph) {
  draw_dataset_points(graph, dataset, C_BLACK);
}

void draw_graph_frame(graph_t* graph) {
  UG_DrawFrame(graph->disp.startx, graph->disp.starty,
               graph->disp.startx + graph->disp.width,
               graph->disp.starty + graph->disp.height,
               graph->disp.outline_colour);
}

void draw_graph(graph_t* graph) {
  // clear_graph(graph);
  draw_graph_frame(graph);

  for (int i = 0; i < graph->dataset_count; i++) {
    draw_dataset_points(graph, graph->dataset[i], graph->dataset[i]->colour);
  }
}

// make y transfrom function callback -- too high soz
void draw_dataset_points(graph_t* graph, graph_dataset_t* dataset,
                         UG_COLOR colour) {
  graph_display_options_t* opts = &graph->disp;

  uint8_t count = 0;
  uint16_t x1, y1, y2;

  uint16_t x_offset =
      ((opts->width - opts->margin) * (uint16_t)100) / graph->size;
  uint16_t height_margin = opts->height - opts->margin;

  graph_point_t* cursor = dataset->head;

  while (cursor != 0) {
    if (cursor->y <= height_margin) {
      // draw left to right
      x1 = _get_x_val(count, x_offset, opts);
      y1 = _get_y_val(cursor->y, opts->starty, opts->height, opts->margin);

      if (opts->draw_point) UG_DrawCircle(x1, y1, opts->circle_radius, colour);

      if (opts->draw_line) {
        if (cursor->next != 0) {
          y2 = _get_y_val(cursor->next->y, opts->starty, opts->height,
                          opts->margin);
          if (dataset->redraw != 0) {
            UG_DrawLine(
                x1, y2,
                _get_x_val(count + 1, x_offset, opts),
                _get_y_val((count == graph->size - 2) ? dataset->finalval
                                                      : cursor->next->next->y,
                           opts->starty, opts->height, opts->margin),
                C_BLACK);
          }
          UG_DrawLine(x1, y1, _get_x_val(count + 1, x_offset, opts), y2, colour);
        }
      }
    }

    count++;
    cursor = cursor->next;
  }
}

void add_point(uint16_t x, uint16_t y, graph_t* graph,
               graph_dataset_t* dataset) {
  dataset->head = prepend_point(x, y, dataset->head);
  if (dataset->count >= graph->size) {
    remove_back(dataset);
  } else {
    dataset->count++;
  }
}

uint16_t _get_x_val(uint8_t count, uint16_t x_offset,
                    graph_display_options_t* opts) {
  return opts->width - opts->margin - (count * x_offset) / 100;
}

uint16_t _get_y_val(uint16_t y, uint16_t starty, uint16_t height,
                    uint16_t margin) {
  return (y <= height - margin) ? (starty + height - margin) - y
                                : starty + margin;
}
