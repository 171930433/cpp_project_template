#pragma once
#include <iguana/util.hpp>
#include <imgui.h>

// ylt_refletable_v
template <typename _T, std::enable_if_t<iguana::ylt_refletable_v<_T>, int> = 0>
inline void TreeViewImpl(std::string_view name, _T& field, ImGuiTreeNodeFlags node_flags);

// plain_v
template <typename _T, std::enable_if_t<iguana::plain_v<_T>, int> = 0>
inline void TreeViewImpl(std::string_view name, _T& field, ImGuiTreeNodeFlags node_flags);

template <typename _T, std::enable_if_t<iguana::plain_v<_T>, int> = 0>
inline void TreeViewImpl(std::string_view name, _T& field, ImGuiTreeNodeFlags node_flags) {
  std::string const filed_name(name);
  char const* str = (std::stringstream() << field).str().c_str();
  ImGui::LabelText(filed_name.c_str(), "%s", str);
}

template <typename _T, std::enable_if_t<iguana::ylt_refletable_v<_T>, int> = 0>
inline void TreeViewImpl(std::string_view name, _T& field, ImGuiTreeNodeFlags node_flags) {
  std::string const filed_name(name);
  ImGui::PushID(filed_name.c_str());

  if (ImGui::TreeNodeEx(filed_name.c_str(), node_flags)) {
    ylt::reflection::for_each(field, [node_flags](auto& field, auto name) { TreeViewImpl(name, field, node_flags); });
    ImGui::TreePop();
  }

  ImGui::PopID();
}

template <typename _T, std::enable_if_t<iguana::ylt_refletable_v<_T>, bool> = true>
inline void TreeView(_T& field, ImGuiTreeNodeFlags node_flags) {
  std::string const filed_name(ylt::reflection::type_string<_T>());
  ImGui::PushID(&field);

  if (ImGui::TreeNodeEx(filed_name.c_str(), node_flags)) {
    ylt::reflection::for_each(field, [node_flags](auto& field, auto name) { TreeViewImpl(name, field, node_flags); });
    ImGui::TreePop();
  }

  ImGui::PopID();
}