
const filters = document.querySelectorAll(".filter-button");
let eventCards = [];
const searchInput = document.querySelector("#event-search");
const noResults = document.querySelector("#no-results");
const modal = document.querySelector("#registration-modal");
const modalEventName = document.querySelector("#modal-event-name");
const eventIdInput = document.querySelector("#event-id");
const registrationForm = document.querySelector("#registration-form");
const formMessage = document.querySelector("#form-message");
const toast = document.querySelector("#success-toast");

let activeFilter = "all";
let lastFocusedElement = null;

const SUPABASE_URL = "https://mmiduvoyvqpnymkfehqo.supabase.co";
const SUPABASE_KEY = "sb_publishable_lX6yaIkBUybaJGy_gtBogg_Odb1d0It";
// Load active events from Supabase
async function loadEvents() {
  const eventGrid = document.querySelector("#event-grid");

  eventGrid.innerHTML = "<p>Loading events...</p>";

  try {
    const response = await fetch(
      `${SUPABASE_URL}/rest/v1/portal_events?select=*&is_active=eq.true&order=event_date.asc`,
      {
        headers: {
          apikey: SUPABASE_KEY,
          Authorization: `Bearer ${SUPABASE_KEY}`
        }
      }
    );

    if (!response.ok) {
      throw new Error(await response.text());
    }

    const events = await response.json();

    eventGrid.innerHTML = "";

    if (events.length === 0) {
      eventGrid.innerHTML = "<p>No events available currently.</p>";
      eventCards = [];
      updateEvents();
      return;
    }

    events.forEach((event, index) => {
      const card = document.createElement("article");

      const category = (event.category || "event")
        .toLowerCase()
        .replace(/[^a-z0-9]+/g, "-")
        .replace(/^-|-$/g, "");

      const styles = [
        ["visual-fest", "visual-fest", "graphic-fest", "tag-social"],
        ["visual-zine", "visual-zine", "graphic-zine", "tag-creative"],
        ["visual-portfolio", "visual-portfolio", "graphic-portfolio", "tag-career"],
        ["visual-yoga", "visual-yoga", "graphic-yoga", "tag-wellness"]
      ];

      const style = styles[index % styles.length];

      const date = event.event_date
        ? new Date(event.event_date)
        : null;

      const day = date && !Number.isNaN(date.getTime())
        ? date.toLocaleDateString("en-IN", { day: "2-digit" })
        : "--";

      const month = date && !Number.isNaN(date.getTime())
        ? date.toLocaleDateString("en-IN", { month: "short" }).toUpperCase()
        : "";

      const time = date && !Number.isNaN(date.getTime())
        ? date.toLocaleTimeString("en-IN", {
          hour: "2-digit",
          minute: "2-digit",
          hour12: true
        })
        : "Time not specified";

      const escapeHTML = (value) =>
        String(value ?? "").replace(/[&<>"']/g, (char) => ({
          "&": "&amp;",
          "<": "&lt;",
          ">": "&gt;",
          '"': "&quot;",
          "'": "&#39;"
        })[char]);

      const title = escapeHTML(event.title);
      const description = escapeHTML(event.description);
      const venue = escapeHTML(event.venue);
      const categoryName = escapeHTML(event.category || "Event");

      const capacityText = event.capacity
        ? `Capacity: ${event.capacity}`
        : "Open registration";

      card.className = "event-card";
      card.dataset.category = category;
      card.dataset.search =
        `${event.title} ${event.description || ""} ${event.category || ""}`;

      card.innerHTML = `
        <div class="event-visual ${style[0]}">
          <span class="event-date">
            <strong>${day}</strong><span>${month}</span>
          </span>

          <span class="visual-label">${categoryName.toUpperCase()}</span>

          <span class="visual-graphic ${style[2]}" aria-hidden="true">
            <i>✳</i><b>${title}</b>
          </span>

          <span class="visual-stamp">JOIN<br>US</span>
        </div>

        <div class="event-info">
          <div class="event-meta">
            <span class="category-tag ${style[3]}">
              ${categoryName.toUpperCase()}
            </span>
            <span>${time} <i>·</i> ${venue}</span>
          </div>

          <h3>${title}</h3>
          <p>${description}</p>

          <div class="event-card-bottom">
            <span class="spots"><i></i>${capacityText}</span>

            <button
              class="register-button"
              type="button"
              data-event="${title}"
              data-event-id="${escapeHTML(event.id)}"
            >
              Save your spot <span aria-hidden="true">↗</span>
            </button>
          </div>
        </div>
      `;

      eventGrid.appendChild(card);
    });

    eventCards = [...document.querySelectorAll(".event-card")];
    updateEvents();

  } catch (error) {
    console.error("Error loading events:", error);
    eventGrid.innerHTML =
      "<p>Unable to load events. Please try again later.</p>";
  }
}
// Filter and search events
function updateEvents() {
  const query = searchInput.value.trim().toLowerCase();
  let visibleCount = 0;

  for (const card of eventCards) {
    const matchesCategory =
      activeFilter === "all" ||
      card.dataset.category === activeFilter;

    const matchesSearch =
      !query ||
      `${card.dataset.search} ${card.textContent}`
        .toLowerCase()
        .includes(query);

    card.hidden = !(matchesCategory && matchesSearch);

    if (!card.hidden) {
      visibleCount++;
    }
  }

  noResults.hidden = visibleCount !== 0;
}

// Category filter buttons
filters.forEach((button) => {
  button.addEventListener("click", () => {
    activeFilter = button.dataset.filter;

    filters.forEach((filter) => {
      const selected = filter === button;
      filter.classList.toggle("is-active", selected);
      filter.setAttribute("aria-pressed", String(selected));
    });

    updateEvents();
  });
});

// Search events
searchInput.addEventListener("input", updateEvents);

// Close registration modal
function closeModal() {
  modal.hidden = true;

  if (lastFocusedElement) {
    lastFocusedElement.focus();
  }
}

// Open registration modal
document.querySelector("#event-grid").addEventListener("click", (event) => {
  const button = event.target.closest(".register-button");

  if (!button) return;

  lastFocusedElement = button;
  modalEventName.textContent = button.dataset.event;
  eventIdInput.value = button.dataset.eventId;
  formMessage.textContent = "";

  modal.hidden = false;
  document.querySelector("#student-name").focus();
});

// Modal close button
document.querySelector(".modal-close").addEventListener("click", closeModal);

// Close modal when clicking outside it
modal.addEventListener("click", (event) => {
  if (event.target === modal) {
    closeModal();
  }
});

// Keyboard shortcuts
document.addEventListener("keydown", (event) => {
  if (event.key === "Escape" && !modal.hidden) {
    closeModal();
  }

  if (
    event.key === "/" &&
    document.activeElement !== searchInput &&
    modal.hidden
  ) {
    event.preventDefault();
    searchInput.focus();
  }
});

// Submit registration to Supabase
registrationForm.addEventListener("submit", async (event) => {
  event.preventDefault();
  formMessage.textContent = "";

  const submitButton = registrationForm.querySelector(
    'button[type="submit"]'
  );

  submitButton.disabled = true;
  submitButton.textContent = "Saving your spot...";

  const formData = new FormData(registrationForm);

  const registration = {
    event_id: eventIdInput.value,
    full_name: formData.get("name").trim(),
    email: formData.get("email").trim(),
    roll_number: formData.get("studentId").trim(),
    department: formData.get("department").trim(),
    status: "pending"
  };

  try {
    const response = await fetch(
      `${SUPABASE_URL}/rest/v1/portal_registrations`,
      {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
          "apikey": SUPABASE_KEY,
          "Authorization": `Bearer ${SUPABASE_KEY}`,
          "Prefer": "return=minimal"
        },
        body: JSON.stringify(registration)
      }
    );

    if (!response.ok) {
      const errorText = await response.text();
      let errorMessage = errorText;

      try {
        const errorData = JSON.parse(errorText);
        errorMessage =
          errorData.message ||
          errorData.error ||
          errorText;
      } catch {
        // Keep the original error text.
      }

      throw new Error(errorMessage || "Registration failed.");
    }

    registrationForm.reset();
    closeModal();

    toast.hidden = false;

    window.setTimeout(() => {
      toast.hidden = true;
    }, 4200);

  } catch (error) {
    formMessage.textContent =
      error.message || "Unable to register. Please try again.";
  } finally {
    submitButton.disabled = false;
    submitButton.innerHTML =
      'Count me in <span aria-hidden="true">↗</span>';
  }
});

// Run event filtering when the page loads
// Load events from Supabase when the page loads
loadEvents();
