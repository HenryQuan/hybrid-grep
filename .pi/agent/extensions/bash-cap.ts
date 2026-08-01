import type { ExtensionAPI } from "@earendil-works/pi-coding-agent";

// Matches trim.c behavior exactly: character-based, env-configurable, same hint format
const DEFAULT_MAX_CHARS = 1000;
const MAX_BUFFER = 65536;

function getMaxChars(): number {
  const env = process.env.TRIM_MAX_CHARS;
  if (env) {
    const v = parseInt(env, 10);
    if (v > 0) return Math.min(v, MAX_BUFFER);
  }
  return DEFAULT_MAX_CHARS;
}

export default function (pi: ExtensionAPI) {
  pi.on("tool_result", async (event) => {
    if (event.toolName !== "bash") return;

    const content = event.content?.[0];
    if (!content || content.type !== "text") return;

    const text = content.text;
    if (!text) return;

    const MAX_CHARS = getMaxChars();
    const totalChars = text.length;

    if (totalChars <= MAX_CHARS) return;

    content.text = `${text.slice(0, MAX_CHARS)}\n--- TRUNCATED (${MAX_CHARS}/${totalChars} chars) ---\nTip: refine further -- narrow the query or use a more specific command.\n`;

    return { content: [content] };
  });
}
