import type { ExtensionAPI } from "@earendil-works/pi-coding-agent";

// Enforce henry-guide 1.2 mechanically with two detectors:
// 1) self-doubt keywords mid-stream -> abort the turn
// 2) text request ("print/explain/...") answered with a tool call -> abort

const CONFUSION_MARKERS = [
  "i'm not sure",
  "i am not sure",
  "can't be sure",
  "not sure whether",
  "not sure if",
  "unsure",
  "uncertain",
  "probably wrong",
  "might be wrong",
  "could be wrong",
  "am i wrong",
  "i may be wrong",
  "i'm confused",
  "i am confused",
  "not certain",
  "i don't know",
];

// Prompts that ask for an answer in text — the agent should reply, not run tools.
const TEXT_REQUEST_RE = /(^|\b)(print|explain|describe|list|say|answer|tell|summarize|how|what|why)\b/i;

export default function (pi: ExtensionAPI) {
  let abortedThisTurn = false;
  let sawToolCallThisTurn = false;
  let textRequestTurn = false;

  pi.on("before_agent_start", (event) => {
    abortedThisTurn = false;
    sawToolCallThisTurn = false;
    textRequestTurn = TEXT_REQUEST_RE.test(event.prompt ?? "");
  });

  pi.on("tool_call", async (event, ctx) => {
    if (abortedThisTurn) return;
    if (sawToolCallThisTurn) return;
    sawToolCallThisTurn = true;

    // User asked for an answer; first move is a tool call -> wrong path.
    if (!textRequestTurn) return;

    abortedThisTurn = true;
    await ctx.abort();
    ctx.ui.notify(
      `User asked for an answer — agent reached for a tool (${event.toolName}). Answer in text instead.`,
      "error"
    );
  });

  pi.on("message_update", async (event, ctx) => {
    if (event.message?.role !== "assistant") return;
    if (abortedThisTurn) return;

    const content = event.message.content;
    const text =
      typeof content === "string"
        ? content
        : JSON.stringify(content ?? "");

    // Ignore code blocks: doubt inside code quotes is not self-doubt.
    const plain = text.replace(/```[\s\S]*?```/g, " ").toLowerCase();

    const hit = CONFUSION_MARKERS.find((m) => plain.includes(m));
    if (!hit) return;

    abortedThisTurn = true;
    await ctx.abort();
    ctx.ui.notify(
      `Agent seems unsure (matched: "${hit}") — please clarify the direction.`,
      "error"
    );
  });
}
