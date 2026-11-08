<!-- Copyright © 2026 陕西禾又科技有限责任公司 -->
<!-- SPDX-License-Identifier: GPL-3.0-only -->

<script setup lang="ts">
import { computed } from "vue";
import { useData, withBase } from "vitepress";

const props = defineProps<{ slug: string; name: string }>();
const { lang } = useData();
const locale = computed(() => lang.value.startsWith("en") ? "en" : "zh");
const source = computed(() => withBase(`/images/components/${locale.value}/${props.slug}.png`));
const alternative = computed(() => locale.value === "en"
  ? `${props.name} component preview`
  : `${props.name} 组件预览`);
</script>

<template>
  <figure class="api-preview">
    <img :src="source" :alt="alternative" width="1280" height="900">
    <figcaption>{{ alternative }}</figcaption>
  </figure>
</template>
