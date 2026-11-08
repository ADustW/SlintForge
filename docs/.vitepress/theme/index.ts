// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

import DefaultTheme from "vitepress/theme";
import type { Theme } from "vitepress";
import ApiScreenshot from "./ApiScreenshot.vue";
import "./style.css";

export default {
  extends: DefaultTheme,
  enhanceApp({ app }) {
    app.component("ApiScreenshot", ApiScreenshot);
  },
} satisfies Theme;
