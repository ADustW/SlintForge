// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { defineConfig, type DefaultTheme } from "vitepress";

type ComponentEntry = {
  name: string;
  category: string;
  slug: string;
};

const components = JSON.parse(
  readFileSync(fileURLToPath(new URL("../api/components.json", import.meta.url)), "utf8"),
) as ComponentEntry[];

const categoryOrder = [
  "basic",
  "inputs",
  "form",
  "navigation",
  "layouts",
  "surfaces",
  "data",
  "feedback",
  "popups",
  "theme",
  "icons",
  "window",
  "other",
  "plot",
  "flow",
];

const categoryLabels: Record<string, { zh: string; en: string }> = {
  basic: { zh: "基础组件", en: "Basic" },
  inputs: { zh: "输入组件", en: "Inputs" },
  form: { zh: "表单组件", en: "Forms" },
  navigation: { zh: "导航组件", en: "Navigation" },
  layouts: { zh: "布局组件", en: "Layouts" },
  surfaces: { zh: "容器组件", en: "Surfaces" },
  data: { zh: "数据展示", en: "Data Display" },
  feedback: { zh: "反馈组件", en: "Feedback" },
  popups: { zh: "弹层组件", en: "Popups" },
  theme: { zh: "主题与材质", en: "Theme and Materials" },
  icons: { zh: "图标", en: "Icons" },
  window: { zh: "窗口", en: "Window" },
  other: { zh: "其他组件", en: "Other" },
  plot: { zh: "ForgePlot", en: "ForgePlot" },
  flow: { zh: "ForgeFlow", en: "ForgeFlow" },
};

function componentGroups(locale: "zh" | "en"): DefaultTheme.SidebarItem[] {
  const prefix = locale === "en" ? "/en" : "";
  return categoryOrder.map((category) => ({
    text: categoryLabels[category][locale],
    collapsed: category === "flow",
    items: components
      .filter((entry) => entry.category === category)
      .sort((left, right) => left.name.localeCompare(right.name))
      .map((entry) => ({
        text: entry.name,
        link: `${prefix}/components/${entry.category}/${entry.slug}`,
      })),
  }));
}

function sidebar(locale: "zh" | "en"): DefaultTheme.SidebarItem[] {
  const prefix = locale === "en" ? "/en" : "";
  const labels = locale === "en"
    ? {
        start: "Getting Started",
        overview: "Overview",
        integration: "Integration",
        components: "Components",
        types: "Shared Types",
        sdk: "SDK Reference",
        rust: "Rust SDK",
        cpp: "C++ SDK",
        migration: "Migration and Compatibility",
      }
    : {
        start: "快速开始",
        overview: "文档说明",
        integration: "接入指南",
        components: "组件 API",
        types: "公共类型",
        sdk: "SDK 参考",
        rust: "Rust SDK",
        cpp: "C++ SDK",
        migration: "迁移与兼容性",
      };
  return [
    {
      text: labels.start,
      items: [
        { text: labels.overview, link: `${prefix}/guide/` },
        { text: labels.integration, link: `${prefix}/guide/integration` },
        { text: labels.migration, link: `${prefix}/guide/migration` },
        { text: locale === "en" ? "Third-party licenses" : "第三方许可证", link: `${prefix}/licenses` },
      ],
    },
    { text: labels.components, items: componentGroups(locale) },
    {
      text: labels.types,
      items: [
        { text: locale === "en" ? "Type Index" : "类型索引", link: `${prefix}/types/` },
        { text: "Navigation / Content", link: `${prefix}/types/navigation` },
        { text: "Widgets / Capture", link: `${prefix}/types/widgets` },
        { text: "Grid", link: `${prefix}/types/grids` },
        { text: "ForgePlot", link: `${prefix}/types/plot` },
        { text: "ForgeFlow", link: `${prefix}/types/flow` },
        { text: "Theme / Typography / Icons", link: `${prefix}/types/theme` },
      ],
    },
    {
      text: labels.sdk,
      items: [
        { text: labels.rust, link: `${prefix}/sdk/rust` },
        { text: labels.cpp, link: `${prefix}/sdk/cpp` },
      ],
    },
  ];
}

const searchTranslations = {
  button: { buttonText: "搜索", buttonAriaLabel: "搜索" },
  modal: {
    displayDetails: "显示详细列表",
    resetButtonTitle: "重置搜索",
    backButtonTitle: "关闭搜索",
    noResultsText: "没有结果",
    footer: {
      selectText: "选择",
      selectKeyAriaLabel: "确认",
      navigateText: "导航",
      navigateUpKeyAriaLabel: "向上",
      navigateDownKeyAriaLabel: "向下",
      closeText: "关闭",
      closeKeyAriaLabel: "Escape",
    },
  },
};

export default defineConfig({
  srcDir: "site",
  outDir: "../build/api-docs-site",
  cacheDir: "../build/api-docs-cache",
  cleanUrls: false,
  ignoreDeadLinks: false,
  title: "SlintForge API",
  description: "SlintForge component and SDK API documentation",
  locales: {
    root: {
      label: "简体中文",
      lang: "zh-CN",
      title: "SlintForge API 文档",
      description: "SlintForge 组件与 Rust/C++ SDK 接口文档",
    },
    en: {
      label: "English",
      lang: "en-US",
      link: "/en/",
      title: "SlintForge API Documentation",
      description: "SlintForge component and Rust/C++ SDK reference",
    },
  },
  themeConfig: {
    sidebar: {
      "/en/": sidebar("en"),
      "/": sidebar("zh"),
    },
    search: {
      provider: "local",
      options: {
        locales: {
          root: { translations: searchTranslations },
        },
      },
    },
    socialLinks: [],
    locales: {
      root: {
        label: "简体中文",
        nav: [
          { text: "组件", link: "/components/basic/forge-card" },
          { text: "公共类型", link: "/types/" },
          { text: "SDK", link: "/sdk/rust" },
        ],
        outline: { label: "本页内容", level: [2, 3] },
        docFooter: { prev: "上一页", next: "下一页" },
        lastUpdated: { text: "最后更新" },
        darkModeSwitchLabel: "外观",
        sidebarMenuLabel: "菜单",
        returnToTopLabel: "返回顶部",
      },
      en: {
        label: "English",
        nav: [
          { text: "Components", link: "/en/components/basic/forge-card" },
          { text: "Types", link: "/en/types/" },
          { text: "SDK", link: "/en/sdk/rust" },
        ],
        outline: { label: "On this page", level: [2, 3] },
      },
    },
  },
});
