#include "coarsen.hpp"
#include "collapse.hpp"
#include "loop.hpp"
#include "refine.hpp"
#include "size.hpp"

namespace Omega_h {

template <typename EdgeLengths, Int dim>
static Read<I8> prevent_overshoot_tmpl(
    Mesh* mesh, AdaptOpts const& opts, LOs cands2edges, Read<I8> cand_codes) {
  CHECK(mesh->dim() == dim);
  auto maxlength = opts.max_length_desired;
  EdgeLengths measurer(mesh);
  auto ev2v = mesh->ask_verts_of(EDGE);
  auto v2e = mesh->ask_up(VERT, EDGE);
  auto ncands = cands2edges.size();
  auto out = Write<I8>(ncands);
  auto f = LAMBDA(LO cand) {
    auto e = cands2edges[cand];
    auto code = cand_codes[cand];
    for (Int eev_col = 0; eev_col < 2; ++eev_col) {
      if (!collapses(code, eev_col)) continue;
      auto v_col = ev2v[e * 2 + eev_col];
      auto eev_onto = 1 - eev_col;
      auto v_onto = ev2v[e * 2 + eev_onto];
      for (auto ve = v2e.a2ab[v_col]; ve < v2e.a2ab[v_col + 1]; ++ve) {
        auto e2 = v2e.ab2b[ve];
        if (e2 == e) continue;
        auto e2_code = v2e.codes[ve];
        auto eev_in = code_which_down(e2_code);
        auto eev_out = 1 - eev_in;
        Few<LO, 2> new_edge;
        new_edge[eev_in] = v_onto;
        new_edge[eev_out] = ev2v[e2 * 2 + eev_out];
        auto length = measurer.measure(new_edge);
        if (length >= maxlength) {
          code = dont_collapse(code, eev_col);
          break;
        }
      }
    }
    out[cand] = code;
  };
  parallel_for(ncands, f);
  return mesh->sync_subset_array(
      EDGE, Read<I8>(out), cands2edges, I8(DONT_COLLAPSE), 1);
}

Read<I8> prevent_overshoot(
    Mesh* mesh, AdaptOpts const& opts, LOs cands2edges, Read<I8> cand_codes) {
  if (mesh->has_tag(VERT, "size") && mesh->dim() == 3) {
    return prevent_overshoot_tmpl<IsoEdgeLengths<3>, 3>(
        mesh, opts, cands2edges, cand_codes);
  }
  if (mesh->has_tag(VERT, "metric") && mesh->dim() == 3) {
    return prevent_overshoot_tmpl<MetricEdgeLengths<3>, 3>(
        mesh, opts, cands2edges, cand_codes);
  }
  if (mesh->has_tag(VERT, "size") && mesh->dim() == 2) {
    return prevent_overshoot_tmpl<IsoEdgeLengths<2>, 2>(
        mesh, opts, cands2edges, cand_codes);
  }
  if (mesh->has_tag(VERT, "metric") && mesh->dim() == 2) {
    return prevent_overshoot_tmpl<MetricEdgeLengths<2>, 2>(
        mesh, opts, cands2edges, cand_codes);
  }
  NORETURN(Read<I8>());
}
}
