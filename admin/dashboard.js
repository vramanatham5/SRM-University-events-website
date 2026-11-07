
const SUPABASE_URL = "https://mmiduvoyvqpnymkfehqo.supabase.co";
const SUPABASE_KEY = "sb_publishable_lX6yaIkBUybaJGy_gtBogg_Odb1d0It";

const db = window.supabase.createClient(
    SUPABASE_URL,
    SUPABASE_KEY
);

const eventForm = document.getElementById("event-form");
const eventMessage = document.getElementById("event-message");

let currentUser;

// Check admin access
async function checkAdmin() {
    const { data: { user }, error } =
        await db.auth.getUser();

    if (error || !user) {
        window.location.href = "index.html";
        return false;
    }

    const { data: profile, error: profileError } =
        await db.from("portal_profiles")
            .select("role")
            .eq("id", user.id)
            .single();

    if (profileError || profile.role !== "admin") {
        await db.auth.signOut();
        window.location.href = "index.html";
        return false;
    }

    currentUser = user;
    return true;
}

// Load events
async function loadEvents() {
    const { data, error } = await db
        .from("portal_events")
        .select("*")
        .order("event_date", { ascending: true });

    if (error) {
        document.getElementById("events-list").textContent = error.message;
        return;
    }

    document.getElementById("event-count").textContent = data.length;

    const container = document.getElementById("events-list");
    container.innerHTML = "";

    data.forEach(event => {
        const item = document.createElement("div");
        item.className = "event-item";

        const title = document.createElement("h3");
        title.textContent = event.title;

        const details = document.createElement("p");
        details.textContent =
            `${event.category} | ${event.venue} | Capacity: ${event.capacity}`;

        const editButton = document.createElement("button");
        editButton.textContent = "Edit";
        editButton.onclick = () => editEvent(event);

        const deleteButton = document.createElement("button");
        deleteButton.textContent = "Delete";
        deleteButton.style.marginLeft = "10px";
        deleteButton.onclick = () => deleteEvent(event.id);

        item.append(title, details, editButton, deleteButton);
        container.appendChild(item);
    });
}

// Add event
eventForm.addEventListener("submit", async (e) => {
    e.preventDefault();

    eventMessage.textContent = "Adding event...";

    const eventData = {
        title: document.getElementById("event-title").value.trim(),
        description: document.getElementById("event-description").value.trim(),
        category: document.getElementById("event-category").value,
        event_date: new Date(
            document.getElementById("event-date").value
        ).toISOString(),
        venue: document.getElementById("event-venue").value.trim(),
        capacity: Number(document.getElementById("event-capacity").value),
        is_active: true,
        created_by: currentUser.id
    };

    const { error } = await db
        .from("portal_events")
        .insert([eventData]);

    if (error) {
        eventMessage.textContent = error.message;
        return;
    }

    eventMessage.textContent = "Event added successfully!";
    eventForm.reset();
    loadEvents();
});

// Edit event
async function editEvent(event) {
    const title = prompt("Enter event title:", event.title);
    if (title === null || !title.trim()) return;

    const venue = prompt("Enter venue:", event.venue || "");
    if (venue === null) return;

    const capacity = prompt("Enter capacity:", event.capacity);
    if (capacity === null || Number(capacity) < 1) return;

    const { error } = await db
        .from("portal_events")
        .update({
            title: title.trim(),
            venue: venue.trim(),
            capacity: Number(capacity)
        })
        .eq("id", event.id);

    if (error) {
        alert(error.message);
        return;
    }

    alert("Event updated successfully!");
    loadEvents();
}

// Delete event (soft delete)
async function deleteEvent(id) {
    if (!confirm("Are you sure you want to deactivate this event?")) {
        return;
    }

    const { error } = await db
        .from("portal_events")
        .update({ is_active: false })
        .eq("id", id);

    if (error) {
        alert(error.message);
        return;
    }

    alert("Event deactivated!");
    loadEvents();
}

// Load registrations
async function loadRegistrations() {
    const { data, error } = await db
        .from("portal_registrations")
        .select("id, full_name, email, roll_number, department, status, registered_at, portal_events(title)")
        .order("registered_at", { ascending: false });

    if (error) {
        document.getElementById("registrations-list").textContent =
            error.message;
        return;
    }

    document.getElementById("registration-count").textContent = data.length;

    const container = document.getElementById("registrations-list");
    container.innerHTML = "";

    data.forEach(registration => {
        const item = document.createElement("div");
        item.className = "registration-item";

        const details = document.createElement("p");
        details.textContent =
            `${registration.full_name || "Unknown"} | ` +
            `${registration.email || ""} | ` +
            `${registration.roll_number || ""} | ` +
            `${registration.department || "N/A"} | ` +
            `${registration.portal_events?.title || "Event"} | ` +
            `${registration.status}`;
        item.appendChild(details);
        container.appendChild(item);
    });
}

// Logout
document.getElementById("logout-btn").addEventListener("click", async () => {
    await db.auth.signOut();
    window.location.href = "index.html";
});

// Start dashboard
async function startDashboard() {
    const allowed = await checkAdmin();

    if (!allowed) return;

    await loadEvents();
    await loadRegistrations();
}

startDashboard();
