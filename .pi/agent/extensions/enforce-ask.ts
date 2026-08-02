import type { ExtensionAPI } from "@earendil-works/pi-coding-agent";

// Enforce henry-guide 1.2 mechanically with four detectors:
// 1) self-doubt keywords mid-stream -> abort the turn
// 2) text request ("print/explain/...") answered with a tool call -> abort
// 3) question prompt -> block write/edit (answer in text)
// 4) no action keyword in prompt -> block write/edit (must ask first)

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

// Questions — user is asking, not requesting a change. Block write/edit.
const QUESTION_RE = /(^|\b)(why|what|where|how|who|which|can you|could you|will you|would you|is it|are there|are you|did you|do you|should i|should we)\b/i;

// Action keywords — user explicitly wants changes. Required for write/edit.
const ACTION_RE = /(^|\b)(update|change|modify|edit|do|go|fix|add|create|remove|delete|scaffold|make|implement|build|rename|move|refactor|rewrite|replace|patch)\b/i;

export default function (pi: ExtensionAPI) {
  let abortedThisTurn = false;
  let sawToolCallThisTurn = false;
  let textRequestTurn = false;
  let questionTurn = false;
  let hasActionKeyword = false;

  pi.on("before_agent_start", (event) => {
    const prompt = event.prompt ?? "";
    abortedThisTurn = false;
    sawToolCallThisTurn = false;
    textRequestTurn = TEXT_REQUEST_RE.test(prompt);
    questionTurn = QUESTION_RE.test(prompt);
    hasActionKeyword = ACTION_RE.test(prompt);
  });

  pi.on("tool_call", async (event, ctx) => {
    if (abortedThisTurn) return;

    // Detector 3+4: block write/edit on questions or missing action keyword.
    if (event.toolName === "write" || event.toolName === "edit") {
      if (questionTurn) {
        abortedThisTurn = true;
        ctx.abort();
        return { block: true, reason: `User asked a question — answer in text, don't ${event.toolName}.` };
      }
      if (!hasActionKeyword) {
        abortedThisTurn = true;
        ctx.abort();
        return { block: true, reason: `No action keyword in prompt — ask before ${event.toolName}.` };
      }
    }

    // Detector 2: first tool call on a text request.
    if (!sawToolCallThisTurn) {
      sawToolCallThisTurn = true;
      if (textRequestTurn) {
        abortedThisTurn = true;
        ctx.abort();
        return { block: true, reason: `User asked for an answer — agent reached for a tool (${event.toolName}). Answer in text instead.` };
      }
    }
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
